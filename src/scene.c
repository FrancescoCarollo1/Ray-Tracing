// Francesco Carollo SM3201419


#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include "scene.h"



Scene *create_empty_scene()
{
    Scene *scene = (Scene *)malloc(sizeof(Scene));
    if (scene == NULL)
    {
        printf("Can't allocate memory for scene\n");
        return NULL;
    }
    // La memoria per le sfere viene allocata in read_scene
    scene->spheres = NULL;
    scene->num_spheres = 0;
    return scene;
}


int read_scene(const char *filename, Scene *scene)
{
    // Apertura del file
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        printf("Error opening file\n");
        return 1;
    }

    // Lettura del viewport
    if (fscanf(file, "VP %f %f %f\n", &scene->viewport.width, &scene->viewport.height, &scene->viewport.depth) != 3)
    {
        printf("Error reading viewport\n");
        return 1;
    }

    // Lettura del colore di sfondo
    if (fscanf(file, "BG %hhu %hhu %hhu\n", &scene->background_color.r, &scene->background_color.g, &scene->background_color.b) != 3)
    {
        printf("Error reading background color\n");
        return 1;
    }

    // Lettura del numero di sfere
    if (fscanf(file, "OBJ_N %d\n", &scene->num_spheres) != 1)
    {
        printf("Error reading number of spheres\n");
        return 1;
    }
    
    // Allocazione della memoria per le sfere
    scene->spheres = (Sphere *)malloc(scene->num_spheres * sizeof(Sphere));
    if (scene->spheres == NULL)
    {
        printf("Can't allocate memory for spheres\n");
        return 1;
    }

    // Lettura delle sfere
    for (int i = 0; i < scene->num_spheres; i++)
    {
       if (fscanf(file, "S %f %f %f %f %hhu %hhu %hhu\n", &scene->spheres[i].center.x, &scene->spheres[i].center.y, &scene->spheres[i].center.z, &scene->spheres[i].radius, &scene->spheres[i].color.r, &scene->spheres[i].color.g, &scene->spheres[i].color.b) != 7)
       {
           printf("Error reading sphere\n");
           return 1;
       }
    }
    return 0;
}

void delete_scene(Scene *s)
{
    free(s->spheres);
    free(s);
}

