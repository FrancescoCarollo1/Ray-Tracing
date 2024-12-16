// Francesco Carollo SM3201419


#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include "scene.h"
#include "vec3.h"


//Define the data types


int read_scene(const char *filename, Scene *scene)
{

    // Open the file
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        perror("Error opening file");
        return 1;
    }

    // Read the viewport
    if (fscanf(file, "VP %f %f %f\n", &scene->viewport.width, &scene->viewport.height, &scene->viewport.depth) != 3)
    {
        perror("Error reading viewport");
        return 1;
    }

    // Read the background color

    if (fscanf(file, "BG %hhu %hhu %hhu\n", &scene->background_color.r, &scene->background_color.g, &scene->background_color.b) != 3)
    {
        perror("Error reading background color");
        return 1;
    }

    // Read the number of spheres
    if (fscanf(file, "OBJ_N %d\n", &scene->num_spheres) != 1)
    {
        perror("Error reading number of spheres");
        return 1;
    }
   
    
    // Allocate memory for the spheres
    scene->spheres = (Sphere *)malloc(scene->num_spheres * sizeof(Sphere));
    if (scene->spheres == NULL)
    {
        perror("Can't allocate memory for spheres");
        return 1;
    }

    // Read the spheres
    for (int i = 0; i < scene->num_spheres; i++)
    {
       if (fscanf(file, "S %f %f %f %f %hhu %hhu %hhu\n", &scene->spheres[i].center.x, &scene->spheres[i].center.y, &scene->spheres[i].center.z, &scene->spheres[i].radius, &scene->spheres[i].color.r, &scene->spheres[i].color.g, &scene->spheres[i].color.b) != 7)
       {
           perror("Error reading sphere");
           return 1;
       }


    }
    return 0;
}

