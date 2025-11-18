// Francesco Carollo SM3201419

#pragma once
#include "vec3.h"
#include "color.h"


// Definizione delle strutture dati
typedef struct 
{
    float width;
    float height;
    float depth;
}Viewport;

typedef struct 
{
    Vec3 origin;
    Vec3 lower_left_corner;
    Vec3 horizontal;    
    Vec3 vertical;
}Camera;

typedef enum {
    DIFFUSE = 0,    
    REFLECTIVE = 1,
    REFRACTIVE = 2,
} MaterialType;

typedef struct 
{
    Vec3 center;
    float radius;
    Color color;
    MaterialType material;
    double mat_param; 
}Sphere;

typedef struct
{
    int num_spheres;
    Sphere *spheres;
    Color background_color;
    Camera camera;
}Scene;

// Dichiarazione delle funzioni
Scene *create_empty_scene();
int read_scene(const char *filename, Scene *scene);
void setup_camera(Scene* scene, Vec3 origin, Vec3 lookat, double vfov, double aspect_ratio);
void delete_scene(Scene *s);