// File: libs/Ray-Tracing/src/scene.c

#include "scene.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h> 

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "vec3.h" 

Scene* create_empty_scene()
{
    Scene *scene = (Scene *)malloc(sizeof(Scene));
    if (scene == NULL) return NULL;
    scene->spheres = NULL;
    scene->num_spheres = 0;
    scene->triangles = NULL;
    scene->num_triangles = 0;
    scene->bvh_root = NULL; 
    return scene;
}

// --- BVH & AABB ---
double fmin_d(double a, double b) { return a < b ? a : b; }
double fmax_d(double a, double b) { return a > b ? a : b; }

AABB get_sphere_aabb(Sphere s) {
    Vec3 r = {s.radius, s.radius, s.radius};
    AABB box;
    box.min = sub(s.center, r);
    box.max = add(s.center, r);
    return box;
}

AABB surrounding_box(AABB box0, AABB box1) {
    Vec3 small = {
        fmin_d(box0.min.x, box1.min.x), fmin_d(box0.min.y, box1.min.y), fmin_d(box0.min.z, box1.min.z)
    };
    Vec3 big = {
        fmax_d(box0.max.x, box1.max.x), fmax_d(box0.max.y, box1.max.y), fmax_d(box0.max.z, box1.max.z)
    };
    return (AABB){small, big};
}

int box_x_compare(const void *a, const void *b) { 
    Sphere *sA = (Sphere *)a; Sphere *sB = (Sphere *)b; return (sA->center.x < sB->center.x) ? -1 : 1; 
}
int box_y_compare(const void *a, const void *b) { 
    Sphere *sA = (Sphere *)a; Sphere *sB = (Sphere *)b; return (sA->center.y < sB->center.y) ? -1 : 1; 
}
int box_z_compare(const void *a, const void *b) { 
    Sphere *sA = (Sphere *)a; Sphere *sB = (Sphere *)b; return (sA->center.z < sB->center.z) ? -1 : 1; 
}

BVHNode* build_bvh_recursive(Sphere *spheres, int n) {
    if (n == 0) return NULL;
    BVHNode *node = (BVHNode*)malloc(sizeof(BVHNode));
    int axis = rand() % 3; 
    
    if (n == 1) {
        node->left = NULL; node->right = NULL; node->sphere = &spheres[0]; node->box = get_sphere_aabb(spheres[0]);
    } else if (n == 2) {
        node->left = build_bvh_recursive(&spheres[0], 1);
        node->right = build_bvh_recursive(&spheres[1], 1);
        node->sphere = NULL;
        node->box = surrounding_box(node->left->box, node->right->box);
    } else {
        if (axis == 0) qsort(spheres, n, sizeof(Sphere), box_x_compare);
        else if (axis == 1) qsort(spheres, n, sizeof(Sphere), box_y_compare);
        else qsort(spheres, n, sizeof(Sphere), box_z_compare);
        int mid = n / 2;
        node->left = build_bvh_recursive(spheres, mid);
        node->right = build_bvh_recursive(spheres + mid, n - mid);
        node->sphere = NULL;
        node->box = surrounding_box(node->left->box, node->right->box);
    }
    return node;
}

bool hit_aabb(const AABB box, const Vec3 ray_origin, const Vec3 ray_dir, double t_min, double t_max) {
    double invD, t0, t1;
    // X
    invD = 1.0 / ray_dir.x; t0 = (box.min.x - ray_origin.x) * invD; t1 = (box.max.x - ray_origin.x) * invD;
    if (invD < 0.0) { double t=t0; t0=t1; t1=t; }
    t_min = t0 > t_min ? t0 : t_min; t_max = t1 < t_max ? t1 : t_max;
    if (t_max <= t_min) return false;
    // Y
    invD = 1.0 / ray_dir.y; t0 = (box.min.y - ray_origin.y) * invD; t1 = (box.max.y - ray_origin.y) * invD;
    if (invD < 0.0) { double t=t0; t0=t1; t1=t; }
    t_min = t0 > t_min ? t0 : t_min; t_max = t1 < t_max ? t1 : t_max;
    if (t_max <= t_min) return false;
    // Z
    invD = 1.0 / ray_dir.z; t0 = (box.min.z - ray_origin.z) * invD; t1 = (box.max.z - ray_origin.z) * invD;
    if (invD < 0.0) { double t=t0; t0=t1; t1=t; }
    t_min = t0 > t_min ? t0 : t_min; t_max = t1 < t_max ? t1 : t_max;
    if (t_max <= t_min) return false;
    return true;
}

void setup_camera(Scene* scene, Vec3 lookfrom, Vec3 lookat, Vec3 vup, double vfov, double aspect_ratio, double aperture, double focus_dist) 
{
    double theta = vfov * M_PI / 180.0;
    double h = tan(theta / 2.0);
    double viewport_height = 2.0 * h * focus_dist;
    double viewport_width = aspect_ratio * viewport_height;
    
    Vec3 w = normalize(sub(lookfrom, lookat));
    Vec3 u = normalize(cross(vup, w)); 
    Vec3 v = cross(w, u);
    
    scene->camera.cam_lookfrom = lookfrom;
    scene->camera.cam_lookat = lookat;
    scene->camera.cam_vup = vup;
    scene->camera.cam_vfov = vfov;
    scene->camera.cam_aspect_ratio = aspect_ratio;
    scene->camera.lens_radius = aperture / 2.0;
    scene->camera.u = u; scene->camera.v = v; scene->camera.w = w;
    scene->camera.horizontal = mul_scalar(u, viewport_width);
    scene->camera.vertical = mul_scalar(v, viewport_height);
    Vec3 hlf_hor = mul_scalar(scene->camera.horizontal, 0.5);
    Vec3 hlf_ver = mul_scalar(scene->camera.vertical, 0.5);
    Vec3 focus_w = mul_scalar(w, focus_dist);
    
    scene->camera.lower_left_corner = sub(lookfrom, hlf_hor);
    scene->camera.lower_left_corner = sub(scene->camera.lower_left_corner, hlf_ver);
    scene->camera.lower_left_corner = sub(scene->camera.lower_left_corner, focus_w);
}

int read_scene(const char *filename, Scene *scene)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL) { printf("Error opening file: %s\n", filename); return 1; }
    printf("Reading scene from %s\n", filename);

    char line[256];
    
    // Legge BG
    while(fgets(line, sizeof(line), file)) {
        if(line[0] == 'B' && line[1] == 'G') {
            sscanf(line, "BG %hhu %hhu %hhu", &scene->background_color.r, &scene->background_color.g, &scene->background_color.b);
            break;
        }
    }
    // Legge CAM
    while(fgets(line, sizeof(line), file)) {
        if(line[0] == 'C' && line[1] == 'A' && line[2] == 'M') {
            sscanf(line, "CAM %f %f %f %f %f %f %f %f %f %lf %lf", 
                &scene->camera.cam_lookfrom.x, &scene->camera.cam_lookfrom.y, &scene->camera.cam_lookfrom.z,
                &scene->camera.cam_lookat.x, &scene->camera.cam_lookat.y, &scene->camera.cam_lookat.z,
                &scene->camera.cam_vup.x, &scene->camera.cam_vup.y, &scene->camera.cam_vup.z,
                &scene->camera.cam_vfov, &scene->camera.cam_aspect_ratio);
            break;
        }
    }

    // Legge OBJ_N e OBJ_T
    rewind(file); // Torniamo all'inizio per cercare le righe ovunque
    while(fgets(line, sizeof(line), file)) {
        if(strncmp(line, "OBJ_N", 5) == 0) {
            sscanf(line, "OBJ_N %d", &scene->num_spheres);
        }
        if(strncmp(line, "OBJ_T", 5) == 0) {
            sscanf(line, "OBJ_T %d", &scene->num_triangles);
        }
    }

    printf("Spheres: %d, Triangles: %d\n", scene->num_spheres, scene->num_triangles);

    // Allocazione
    if (scene->num_spheres > 0) scene->spheres = (Sphere *)malloc(scene->num_spheres * sizeof(Sphere));
    if (scene->num_triangles > 0) scene->triangles = (Triangle *)malloc(scene->num_triangles * sizeof(Triangle));

    // Parsing Dati
    rewind(file);
    int s_idx = 0;
    int t_idx = 0;
    
    while(fgets(line, sizeof(line), file)) {
        if (line[0] == 'S' && s_idx < scene->num_spheres) {

            float x,y,z, r_val;
            double param;
            int mat;
            sscanf(line, "S %f %f %f %f %hhu %hhu %hhu %d %lf", 
                &x, &y, &z, &r_val, 
                &scene->spheres[s_idx].color.r, &scene->spheres[s_idx].color.g, &scene->spheres[s_idx].color.b, 
                &mat, &param);
            
            scene->spheres[s_idx].center.x = x;
            scene->spheres[s_idx].center.y = y;
            scene->spheres[s_idx].center.z = z;
            scene->spheres[s_idx].radius = r_val;
            scene->spheres[s_idx].material = mat;
            scene->spheres[s_idx].mat_param = param;
            s_idx++;
        }
        else if (line[0] == 'T' && t_idx < scene->num_triangles) {
            // Leggiamo i 3 vertici
            float v0x, v0y, v0z, v1x, v1y, v1z, v2x, v2y, v2z;
            double param;
            int mat;
            sscanf(line, "T %f %f %f %f %f %f %f %f %f %hhu %hhu %hhu %d %lf",
                &v0x, &v0y, &v0z, &v1x, &v1y, &v1z, &v2x, &v2y, &v2z,
                &scene->triangles[t_idx].color.r, &scene->triangles[t_idx].color.g, &scene->triangles[t_idx].color.b,
                &mat, &param);

            scene->triangles[t_idx].v0.x = v0x; scene->triangles[t_idx].v0.y = v0y; scene->triangles[t_idx].v0.z = v0z;
            scene->triangles[t_idx].v1.x = v1x; scene->triangles[t_idx].v1.y = v1y; scene->triangles[t_idx].v1.z = v1z;
            scene->triangles[t_idx].v2.x = v2x; scene->triangles[t_idx].v2.y = v2y; scene->triangles[t_idx].v2.z = v2z;
            scene->triangles[t_idx].material = (MaterialType)mat;
            scene->triangles[t_idx].mat_param = param;
            
            // DEBUG PRINT
            printf("Loaded TRI %d: V0(%.2f %.2f %.2f)\n", t_idx, v0x, v0y, v0z);
            t_idx++;
        }
    }
    fclose(file); 

    // BVH e Camera Init
    if (scene->num_spheres > 0) {
        printf("Building BVH...\n");
        scene->bvh_root = build_bvh_recursive(scene->spheres, scene->num_spheres);
    } else {
        scene->bvh_root = NULL;
    }

    // Distanza focale di default
    double dist = sqrt(pow(scene->camera.cam_lookfrom.x - scene->camera.cam_lookat.x, 2) +
                       pow(scene->camera.cam_lookfrom.y - scene->camera.cam_lookat.y, 2) +
                       pow(scene->camera.cam_lookfrom.z - scene->camera.cam_lookat.z, 2));

    setup_camera(scene, scene->camera.cam_lookfrom, scene->camera.cam_lookat, scene->camera.cam_vup, 
                 scene->camera.cam_vfov, scene->camera.cam_aspect_ratio, 0.0, dist); 

    return 0;
}

void delete_scene(Scene *s) {
    if (s == NULL) return;  
    free(s->spheres);
    free(s->triangles);
    free(s);
}