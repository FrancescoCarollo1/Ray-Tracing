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
    Vec3 center;
    float radius;
    Color color;
}Sphere;

typedef struct
{
    Color background_color;
    Viewport viewport;
    int num_spheres;
    Sphere *spheres;
}Scene;

// Dichiarazione delle funzioni
Scene *create_empty_scene();
int read_scene(const char *filename, Scene *scene);
void delete_scene(Scene *s);