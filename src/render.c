// File: libs/Ray-Tracing/src/render.c
// Francesco Carollo SM3201419

#include "render.h"
#include "vec3.h"
#include "color.h"
#include "ppm.h"
#include "scene.h"
#include <math.h>
#include <omp.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#ifndef EPSILON
#define EPSILON 0.001
#endif

// --- RANDOM UTILS ---
uint32_t xorshift32(uint32_t *state) {
    uint32_t x = *state;
    x ^= x << 13; x ^= x >> 17; x ^= x << 5;
    *state = x;
    return x;
}
double random_double_fast(uint32_t *state) { return (double)(xorshift32(state)) / 4294967295.0; }
double random_double_range(uint32_t *state, double min, double max) { return min + (max - min) * random_double_fast(state); }

Vec3 random_in_unit_sphere(uint32_t *state) {
    while (1) {
        Vec3 p;
        p.x = random_double_range(state, -1, 1);
        p.y = random_double_range(state, -1, 1);
        p.z = random_double_range(state, -1, 1);
        if (prodotto_scalare(p, p) < 1) return p;
    }
}
Vec3 random_unit_vector(uint32_t *state) { return normalize(random_in_unit_sphere(state)); }
Vec3 random_in_unit_disk(uint32_t *state) {
    while (true) {
        Vec3 p;
        p.x = random_double_range(state, -1, 1);
        p.y = random_double_range(state, -1, 1);
        p.z = 0;
        if ((p.x*p.x + p.y*p.y) < 1) return p;
    }
}

// --- VECTOR UTILS ---
Vec3 color_to_vec3(Color c) { return (Vec3){(double)c.r / 255.0, (double)c.g / 255.0, (double)c.b / 255.0}; }
Vec3 mul_vec3(Vec3 v1, Vec3 v2) { return (Vec3){v1.x * v2.x, v1.y * v2.y, v1.z * v2.z}; }
Vec3 lerp(Vec3 start, Vec3 end, double t) { return add(mul_scalar(start, 1.0 - t), mul_scalar(end, t)); }

// --- FISICA ---
Vec3 reflect(Vec3 v, Vec3 n) {
    double dot_vn = 2.0 * prodotto_scalare(v, n);
    return sub(v, mul_scalar(n, dot_vn));
}
Vec3 refract(Vec3 uv, Vec3 n, double etai_over_etat) {
    double cos_theta = fmin(prodotto_scalare(mul_scalar(uv, -1.0), n), 1.0);
    Vec3 r_out_perp = mul_scalar(add(uv, mul_scalar(n, cos_theta)), etai_over_etat);
    Vec3 r_out_parallel = mul_scalar(n, -sqrt(fabs(1.0 - prodotto_scalare(r_out_perp, r_out_perp))));
    return add(r_out_perp, r_out_parallel);
}
double reflectance(double cosine, double ref_idx) {
    double r0 = (1 - ref_idx) / (1 + ref_idx);
    r0 = r0 * r0;
    return r0 + (1 - r0) * pow((1 - cosine), 5);
}

// --- INTERSEZIONI ---

// Sfera
double distanza_sfera(Vec3 ray_dir, Vec3 ray_origin, Sphere sphere) {
    Vec3 oc = sub(ray_origin, sphere.center);
    double a = prodotto_scalare(ray_dir, ray_dir);
    double b = 2.0 * prodotto_scalare(oc, ray_dir);
    double c = prodotto_scalare(oc, oc) - (double)sphere.radius * (double)sphere.radius;
    double delta = (b * b) - (4.0 * a * c);
    if (delta < 0) return INFINITY;
    double min_t = 0.001; 
    double sqrt_delta = sqrt(delta);
    double t1 = (-b - sqrt_delta) / (2.0 * a);
    if (t1 > min_t) return t1;
    double t2 = (-b + sqrt_delta) / (2.0 * a);
    if (t2 > min_t) return t2;
    return INFINITY;
}

// Triangolo 
double hit_triangle(Vec3 ray_origin, Vec3 ray_dir, Triangle tri, Vec3 *out_normal) {
    const double EPSILON_TRI = 0.0000001;
    Vec3 edge1 = sub(tri.v1, tri.v0);
    Vec3 edge2 = sub(tri.v2, tri.v0);
    Vec3 h = cross(ray_dir, edge2); 
    double a = prodotto_scalare(edge1, h);
    
    if (a > -EPSILON_TRI && a < EPSILON_TRI) return INFINITY; // Raggio parallelo
    
    double f = 1.0 / a;
    Vec3 s = sub(ray_origin, tri.v0);
    double u = f * prodotto_scalare(s, h);
    if (u < 0.0 || u > 1.0) return INFINITY;
    
    Vec3 q = cross(s, edge1);
    double v = f * prodotto_scalare(ray_dir, q);
    if (v < 0.0 || u + v > 1.0) return INFINITY;
    
    double t = f * prodotto_scalare(edge2, q);
    if (t > 0.001) {
        *out_normal = normalize(cross(edge1, edge2)); // Normale piatta
        return t;
    }
    return INFINITY;
}

// BVH Traversal 
// BVH Traversal Generico
bool hit_bvh_node(BVHNode *node, Vec3 r_orig, Vec3 r_dir, double t_min, double t_max, 
                  double *closest_t, int *hit_index, Scene *scene, int mode) 
{
    // 1. Check Box
    if (!hit_aabb(node->box, r_orig, r_dir, t_min, t_max)) return false;

    // 2. Check Foglia
    bool hit_leaf = false;
    
    // Caso Sfera
    if (node->sphere != NULL) {
        double t = distanza_sfera(r_dir, r_orig, *(node->sphere));
        if (t < *closest_t && t > 0.001) {
            *closest_t = t;
            *hit_index = (int)(node->sphere - scene->spheres); // Indice array
            return true;
        }
        return false;
    }
    
    // Caso Triangolo
    if (node->triangle != NULL) {
        Vec3 dummy_norm; // La normale la ricalcoliamo dopo
        double t = hit_triangle(r_orig, r_dir, *(node->triangle), &dummy_norm);
        if (t < *closest_t && t > 0.001) {
            *closest_t = t;
            *hit_index = (int)(node->triangle - scene->triangles);
            return true;
        }
        return false;
    }

    // 3. Check Figli
    bool hit_left = hit_bvh_node(node->left, r_orig, r_dir, t_min, t_max, closest_t, hit_index, scene, mode);
    bool hit_right = hit_bvh_node(node->right, r_orig, r_dir, t_min, (hit_left ? *closest_t : t_max), closest_t, hit_index, scene, mode);

    return hit_left || hit_right;
}



Vec3 colore_raggio_ricorsivo(Vec3 ray_origin, Vec3 ray_dir, Scene *scene, int depth, uint32_t *seed)
{
    if (depth <= 0) return (Vec3){0.0, 0.0, 0.0};

    double closest_t = INFINITY;
    int hit_idx_sph = -1;
    int hit_idx_tri = -1;

    // 1. BVH Sfere
    if (scene->bvh_root) {
        hit_bvh_node(scene->bvh_root, ray_origin, ray_dir, 0.001, INFINITY, &closest_t, &hit_idx_sph, scene, 0);
    } else { // Fallback
        for(int i=0; i<scene->num_spheres; i++) {
            double t = distanza_sfera(ray_dir, ray_origin, scene->spheres[i]);
            if(t < closest_t) { closest_t = t; hit_idx_sph = i; }
        }
    }

    // 2. BVH Triangoli
    if (scene->bvh_tri_root) {
        // Resettiamo indice hit sfera se troviamo un triangolo più vicino
        int temp_idx = -1; 
        if (hit_bvh_node(scene->bvh_tri_root, ray_origin, ray_dir, 0.001, closest_t, &closest_t, &temp_idx, scene, 1)) {
            hit_idx_tri = temp_idx;
            hit_idx_sph = -1; 
        }
    } else { // Fallback loop
        Vec3 dummy;
        for(int i=0; i<scene->num_triangles; i++) {
            double t = hit_triangle(ray_origin, ray_dir, scene->triangles[i], &dummy);
            if(t < closest_t) { closest_t = t; hit_idx_tri = i; hit_idx_sph = -1; }
        }
    }

    //  HIT PROCESSING 
    if (hit_idx_sph != -1 || hit_idx_tri != -1) {
        Vec3 P = add(ray_origin, mul_scalar(ray_dir, closest_t));
        Vec3 N;
        MaterialType mat;
        Color col;
        double mat_param;

        if (hit_idx_sph != -1) {
            Sphere obj = scene->spheres[hit_idx_sph];
            N = normalize(sub(P, obj.center));
            mat = obj.material; col = obj.color; mat_param = obj.mat_param;
        } else {
            Triangle obj = scene->triangles[hit_idx_tri];
            Vec3 e1 = sub(obj.v1, obj.v0);
            Vec3 e2 = sub(obj.v2, obj.v0);
            N = normalize(cross(e1, e2)); // Flat shading normale
            mat = obj.material; col = obj.color; mat_param = obj.mat_param;
        }
        
        
        bool front_face = prodotto_scalare(ray_dir, N) < 0;
        Vec3 outward_normal = front_face ? N : mul_scalar(N, -1.0);
        
        Vec3 emitted = {0,0,0};
        Vec3 attenuation = {0,0,0};
        Vec3 scattered_origin, scattered_dir;
        bool did_scatter = false;

        if (mat == EMISSIVE) {
            emitted = mul_scalar(color_to_vec3(col), mat_param);
            did_scatter = false;
        }
        else if (mat == DIFFUSE) {
            Vec3 rnd = random_unit_vector(seed);
            scattered_dir = normalize(add(outward_normal, rnd));
            if ((fabs(scattered_dir.x)<1e-8)&&(fabs(scattered_dir.y)<1e-8)&&(fabs(scattered_dir.z)<1e-8)) scattered_dir = outward_normal;
            scattered_origin = add(P, mul_scalar(outward_normal, EPSILON));
            attenuation = color_to_vec3(col);
            did_scatter = true;
        }
        else if (mat == REFLECTIVE) {
             Vec3 reflected = reflect(normalize(ray_dir), outward_normal);
             scattered_dir = normalize(add(reflected, mul_scalar(random_in_unit_sphere(seed), mat_param)));
             scattered_origin = add(P, mul_scalar(outward_normal, EPSILON));
             attenuation = color_to_vec3(col);
             did_scatter = (prodotto_scalare(scattered_dir, outward_normal) > 0);
        }
        else if (mat == REFRACTIVE) {
             attenuation = (Vec3){1,1,1};
             double ratio = front_face ? (1.0/mat_param) : mat_param;
             Vec3 unit_dir = normalize(ray_dir);
             double cos_theta = fmin(prodotto_scalare(mul_scalar(unit_dir, -1), outward_normal), 1.0);
             double sin_theta = sqrt(1.0 - cos_theta*cos_theta);
             bool cannot = ratio * sin_theta > 1.0;
             if (cannot || reflectance(cos_theta, ratio) > random_double_fast(seed)) scattered_dir = reflect(unit_dir, outward_normal);
             else scattered_dir = refract(unit_dir, outward_normal, ratio);
             
             double dot_out = prodotto_scalare(scattered_dir, outward_normal);
             double push = (dot_out > 0) ? 0.002 : -0.002;
             scattered_origin = add(P, mul_scalar(outward_normal, push));
             did_scatter = true;
        }

        if (did_scatter) {
            return add(emitted, mul_vec3(attenuation, colore_raggio_ricorsivo(scattered_origin, scattered_dir, scene, depth-1, seed)));
        } else {
            return emitted;
        }
    }

    // Sky
    if (scene->background_color.r == 0 && scene->background_color.g == 0 && scene->background_color.b == 0) return (Vec3){0,0,0};
    Vec3 unit = normalize(ray_dir);
    double t = 0.5 * (unit.y + 1.0);
    return lerp((Vec3){1,1,1}, (Vec3){0.5, 0.7, 1.0}, t);
}


// --- MAIN RENDER LOOP ---
void omp_render_scene(Scene *scene, Color *pixel_out, int width, int height, int samples_per_pixel, int frame_index)
{
    const int MAX_DEPTH = 50; 
    Camera cam = scene->camera;

#pragma omp parallel for collapse(2) schedule(dynamic)
    for (int j = 0; j < height; j++)
    {
        for (int i = 0; i < width; i++)
        {
            uint32_t seed = (i * 98765) ^ (j * 43210) ^ (omp_get_thread_num() * 719393) ^ (frame_index * 334455);
            Vec3 total_color_vec = {0.0, 0.0, 0.0};

            for (int s = 0; s < samples_per_pixel; s++)
            {
                Vec3 offset = {0, 0, 0};
                if (cam.lens_radius > 0.0) {
                    Vec3 rd = mul_scalar(random_in_unit_disk(&seed), cam.lens_radius);
                    Vec3 u_offset = mul_scalar(cam.u, rd.x);
                    Vec3 v_offset = mul_scalar(cam.v, rd.y);
                    offset = add(u_offset, v_offset);
                }

                double u = ((double)i + random_double_fast(&seed)) / (width - 1);
                double v = ((double)(height - 1 - j) + random_double_fast(&seed)) / (height - 1);

                Vec3 h_offset = mul_scalar(cam.horizontal, u);
                Vec3 v_offset = mul_scalar(cam.vertical, v);
                Vec3 target = add(cam.lower_left_corner, h_offset);
                target = add(target, v_offset);

                Vec3 ray_origin = add(cam.cam_lookfrom, offset);
                Vec3 ray_direction = sub(target, ray_origin);

                Vec3 sample_color = colore_raggio_ricorsivo(ray_origin, ray_direction, scene, MAX_DEPTH, &seed);

                if (sample_color.x != sample_color.x) sample_color.x = 0.0;
                if (sample_color.y != sample_color.y) sample_color.y = 0.0;
                if (sample_color.z != sample_color.z) sample_color.z = 0.0;

                total_color_vec.x += sample_color.x;
                total_color_vec.y += sample_color.y;
                total_color_vec.z += sample_color.z;
            }

            double scale = 1.0 / samples_per_pixel;
            total_color_vec.x *= scale;
            total_color_vec.y *= scale;
            total_color_vec.z *= scale;

            total_color_vec.x = sqrt(total_color_vec.x);
            total_color_vec.y = sqrt(total_color_vec.y);
            total_color_vec.z = sqrt(total_color_vec.z);

            Color final_color;
            final_color.r = (uint8_t)(fmin(total_color_vec.x, 0.999) * 256);
            final_color.g = (uint8_t)(fmin(total_color_vec.y, 0.999) * 256);
            final_color.b = (uint8_t)(fmin(total_color_vec.z, 0.999) * 256);

            pixel_out[i + j * width] = final_color;
        }
    }
}