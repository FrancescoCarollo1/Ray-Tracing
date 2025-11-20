// Francesco Carollo SM3201419

#pragma once
#include "vec3.h"
#include "color.h"
#include <stdbool.h>

// Definizione delle strutture dati
typedef struct 
{
    float width;
    float height;
    float depth;
}Viewport;

typedef struct 
{
    Vec3 cam_lookfrom;
    Vec3 cam_lookat;
    Vec3 cam_vup;
    double cam_vfov;
    double cam_aspect_ratio;

    Vec3 lower_left_corner;
    Vec3 horizontal;    
    Vec3 vertical;
    
    //---Vettori di base della camera---
    Vec3 u; 
    Vec3 v; 
    Vec3 w; 

    double lens_radius; 
} Camera;

typedef enum {
    DIFFUSE = 0,    
    REFLECTIVE = 1,
    REFRACTIVE = 2,
    EMISSIVE = 3  
} MaterialType;

typedef struct 
{
    Vec3 center;
    float radius;
    Color color;
    MaterialType material;
    double mat_param; 
}Sphere;

typedef struct {
    Vec3 v0;
    Vec3 v1;
    Vec3 v2;
    Color color;
    MaterialType material;
    double mat_param;
} Triangle;

// 1. Axis Aligned Bounding Box (AABB)
typedef struct {
    Vec3 min;
    Vec3 max;
} AABB;

// 2. Nodo dell'albero
typedef struct BVHNode {
    AABB box;               
    struct BVHNode *left;       
    struct BVHNode *right;       
    Sphere *sphere;             
} BVHNode;

typedef struct
{
    int num_spheres;
    Sphere *spheres;

    int num_triangles;
    Triangle *triangles;
    
    Color background_color;

    Camera camera;

    BVHNode *bvh_root; 
}Scene;

// Dichiarazione delle funzioni
Scene *create_empty_scene();
int read_scene(const char *filename, Scene *scene);
void setup_camera(Scene* scene, Vec3 lookfrom, Vec3 lookat, Vec3 vup, double vfov, double aspect_ratio, double aperture, double focus_dist);
void delete_scene(Scene *s);

bool hit_aabb(const AABB box, const Vec3 ray_origin, const Vec3 ray_dir, double t_min, double t_max);