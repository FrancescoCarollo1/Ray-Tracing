// File: libs/Ray-Tracing/src/scene.c

#include "scene.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "vec3.h" 

Scene* create_empty_scene()
{
    Scene *scene = (Scene *)malloc(sizeof(Scene));
    if (scene == NULL) {
        printf("Can't allocate memory for scene\n");
        return NULL;
    }
    scene->spheres = NULL;
    scene->num_spheres = 0;
    scene->bvh_root = NULL; 
    return scene;
}

// --- IMPLEMENTAZIONE BVH E AABB ---

// Calcola il min e max tra due valori
double fmin_d(double a, double b) { return a < b ? a : b; }
double fmax_d(double a, double b) { return a > b ? a : b; }

// Crea una box che circonda una sfera
AABB get_sphere_aabb(Sphere s) {
    Vec3 r = {s.radius, s.radius, s.radius};
    AABB box;
    box.min = sub(s.center, r);
    box.max = add(s.center, r);
    return box;
}

// Unisce due box in una più grande che le contiene entrambe
AABB surrounding_box(AABB box0, AABB box1) {
    Vec3 small = {
        fmin_d(box0.min.x, box1.min.x),
        fmin_d(box0.min.y, box1.min.y),
        fmin_d(box0.min.z, box1.min.z)
    };
    Vec3 big = {
        fmax_d(box0.max.x, box1.max.x),
        fmax_d(box0.max.y, box1.max.y),
        fmax_d(box0.max.z, box1.max.z)
    };
    return (AABB){small, big};
}

// Funzioni di comparazione per qsort (ordinano le sfere lungo gli assi)
int box_compare(const void *a, const void *b, int axis) {
    Sphere *sphereA = (Sphere *)a;
    Sphere *sphereB = (Sphere *)b;
    float minA = (axis == 0) ? sphereA->center.x : (axis == 1) ? sphereA->center.y : sphereA->center.z;
    float minB = (axis == 0) ? sphereB->center.x : (axis == 1) ? sphereB->center.y : sphereB->center.z;
    return (minA < minB) ? -1 : 1;
}

int box_x_compare(const void *a, const void *b) { return box_compare(a, b, 0); }
int box_y_compare(const void *a, const void *b) { return box_compare(a, b, 1); }
int box_z_compare(const void *a, const void *b) { return box_compare(a, b, 2); }

// ---  COSTRUTTORE  ---
BVHNode* build_bvh_recursive(Sphere *spheres, int n) {
    BVHNode *node = (BVHNode*)malloc(sizeof(BVHNode));
    
    int axis = rand() % 3; 
    
    if (n == 1) {
        node->left = NULL;
        node->right = NULL;
        node->sphere = &spheres[0]; 
        node->box = get_sphere_aabb(spheres[0]);
    } 
    else if (n == 2) {
        node->left = build_bvh_recursive(&spheres[0], 1);
        node->right = build_bvh_recursive(&spheres[1], 1);
        node->sphere = NULL;
        node->box = surrounding_box(node->left->box, node->right->box);
    } 
    else {
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

// Funzione Intersezione AABB
bool hit_aabb(const AABB box, const Vec3 ray_origin, const Vec3 ray_dir, double t_min, double t_max) {
    double invD = 1.0 / ray_dir.x;
    double t0 = (box.min.x - ray_origin.x) * invD;
    double t1 = (box.max.x - ray_origin.x) * invD;
    if (invD < 0.0) { double temp = t0; t0 = t1; t1 = temp; }
    t_min = t0 > t_min ? t0 : t_min;
    t_max = t1 < t_max ? t1 : t_max;
    if (t_max <= t_min) return false;

    invD = 1.0 / ray_dir.y;
    t0 = (box.min.y - ray_origin.y) * invD;
    t1 = (box.max.y - ray_origin.y) * invD;
    if (invD < 0.0) { double temp = t0; t0 = t1; t1 = temp; }
    t_min = t0 > t_min ? t0 : t_min;
    t_max = t1 < t_max ? t1 : t_max;
    if (t_max <= t_min) return false;

    invD = 1.0 / ray_dir.z;
    t0 = (box.min.z - ray_origin.z) * invD;
    t1 = (box.max.z - ray_origin.z) * invD;
    if (invD < 0.0) { double temp = t0; t0 = t1; t1 = temp; }
    t_min = t0 > t_min ? t0 : t_min;
    t_max = t1 < t_max ? t1 : t_max;
    if (t_max <= t_min) return false;

    return true;
}

// Setup della camera nella scena
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

    scene->camera.u = u;
    scene->camera.v = v;
    scene->camera.w = w;

    scene->camera.horizontal = mul_scalar(u, viewport_width);
    scene->camera.vertical = mul_scalar(v, viewport_height);

    Vec3 hlf_hor = mul_scalar(scene->camera.horizontal, 0.5);
    Vec3 hlf_ver = mul_scalar(scene->camera.vertical, 0.5);
    Vec3 focus_w = mul_scalar(w, focus_dist);
    
    scene->camera.lower_left_corner = sub(lookfrom, hlf_hor);
    scene->camera.lower_left_corner = sub(scene->camera.lower_left_corner, hlf_ver);
    scene->camera.lower_left_corner = sub(scene->camera.lower_left_corner, focus_w);
}

// Caricamento scena 
int read_scene(const char *filename, Scene *scene)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file: %s\n", filename);
        return 1;
    }

    printf("Reading scene from %s\n", filename);

    if (fscanf(file, "BG %hhu %hhu %hhu\n", &scene->background_color.r, &scene->background_color.g, &scene->background_color.b) != 3) {
        printf("Error reading background color\n"); fclose(file); return 1;
    }

    if (fscanf(file, "CAM %f %f %f %f %f %f %f %f %f %lf %lf\n", 
        &scene->camera.cam_lookfrom.x, &scene->camera.cam_lookfrom.y, &scene->camera.cam_lookfrom.z,
        &scene->camera.cam_lookat.x, &scene->camera.cam_lookat.y, &scene->camera.cam_lookat.z,
        &scene->camera.cam_vup.x, &scene->camera.cam_vup.y, &scene->camera.cam_vup.z,
        &scene->camera.cam_vfov,
        &scene->camera.cam_aspect_ratio) != 11) {
        printf("Error reading camera parameters\n"); fclose(file); return 1;
    }

    if (fscanf(file, "OBJ_N %d\n", &scene->num_spheres) != 1) {
        printf("Error reading number of spheres\n"); fclose(file); return 1;
    }

    scene->spheres = (Sphere *)malloc(scene->num_spheres * sizeof(Sphere));
    if (scene->spheres == NULL) {
        printf("Can't allocate memory for spheres\n"); fclose(file); return 1;
    }

    for (int i = 0; i < scene->num_spheres; i++)
    {
       if (fscanf(file, "S %f %f %f %f %hhu %hhu %hhu %d %lf\n", 
          &scene->spheres[i].center.x,
          &scene->spheres[i].center.y, 
          &scene->spheres[i].center.z,
          &scene->spheres[i].radius, 
          &scene->spheres[i].color.r, 
          &scene->spheres[i].color.g, 
          &scene->spheres[i].color.b, 
          &scene->spheres[i].material, 
          &scene->spheres[i].mat_param) != 9)
       {
           printf("Error reading sphere index %d\n", i);
           fclose(file);
           return 1;
       }
       
       // Debug print)
       /*
       printf("Sphere %d loaded: Mat=%d, Param=%.2f\n", 
              i, 
              scene->spheres[i].material, 
              scene->spheres[i].mat_param);
        */
    }
    printf("Finished reading spheres\n");
    fclose(file); 

    // --- COSTRUZIONE BVH  ---
    printf("Building BVH for %d spheres...\n", scene->num_spheres);
    scene->bvh_root = build_bvh_recursive(scene->spheres, scene->num_spheres);
    printf("BVH Built successfully.\n");

    // --- SETUP INIZIALE CAMERA ---
    double dist = sqrt(pow(scene->camera.cam_lookfrom.x - scene->camera.cam_lookat.x, 2) +
                       pow(scene->camera.cam_lookfrom.y - scene->camera.cam_lookat.y, 2) +
                       pow(scene->camera.cam_lookfrom.z - scene->camera.cam_lookat.z, 2));

    setup_camera(scene, 
                 scene->camera.cam_lookfrom, 
                 scene->camera.cam_lookat, 
                 scene->camera.cam_vup, 
                 scene->camera.cam_vfov, 
                 scene->camera.cam_aspect_ratio,
                 0.0,   
                 dist); 

    return 0;
}

void delete_scene(Scene *s)
{
    if (s == NULL) return;  
    free(s->spheres);
    free(s);
}