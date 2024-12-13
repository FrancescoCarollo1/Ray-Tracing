//Francesco Carollo SM3201419

#include <stdint.h>



typedef struct 
{
    float width;
    float height;
    float depth;
}Viewport;

typedef struct
{
   uint8_t r;
   uint8_t g;
   uint8_t b;
}Color;

typedef struct 
{
    float x;
    float y;
    float z;
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



int read_scene(const char *filename, Scene *scene);