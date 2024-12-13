// Francesco Carollo SM3201419


#include <stdio.h>
#include <stddef.h>
#include <math.h>
#include <omp.h>
#include <stdlib.h>


//Define the data types

typedef struct 
{
    float width;
    float height;
    float depth;
}Viewport;

typedef struct
{
   int r;
   int g;
   int b;
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
    Viewport viewport;
    int num_spheres;
    Sphere *spheres;
}Scene;


void read_scene(const char *filename, Scene *scene)
{

    // Open the file
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        perror("Error opening file");
        return;
    }

    // Read the viewport
    if (fscanf(file, "%f %f %f\n", &scene->viewport.width, &scene->viewport.height, &scene->viewport.depth) != 3)
    {
        perror("Error reading viewport");
        return;
    }

    // Read the number of spheres
    if (fscanf(file, "%d\n", &scene->num_spheres) < 0)
    {
        perror("The number of spheres is negative");
        return;
    }
    
    // Allocate memory for the spheres
    scene->spheres = (Sphere *)malloc(scene->num_spheres * sizeof(Sphere));
    if (scene->spheres == NULL)
    {
        perror("Can't allocate memory for spheres");
        return;
    }

    // Read the spheres
    for (int i = 0; i < scene->num_spheres; i++)
    {
       if (fscanf(file, "%f %f %f %f %d %d %d\n", &scene->spheres[i].x, &scene->spheres[i].y, &scene->spheres[i].z, &scene->spheres[i].radius, &scene->spheres[i].color.r, &scene->spheres[i].color.g, &scene->spheres[i].color.b) != 7)
       {
           perror("Error reading sphere");
           return;
       }


    }
}

