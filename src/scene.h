//Francesco Carollo SM3201419

#pragma once
#include <stdint.h>
#include "vec3.h"

//Define the data types

typedef struct 
{
    float width;
    float height;
    float depth;
}Viewport;

typedef struct __attribute__ ((packed))
{
   uint8_t r;
   uint8_t g;
   uint8_t b;
}Color;

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


//Function prototypes
Scene *create_empty_scene();
int read_scene(const char *filename, Scene *scene);
void delete_scene(Scene *s);