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
    return scene;
}

int read_scene(const char *filename, Scene *scene)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file\n");
        return 1;
    }

    // Lettura del colore di sfondo
    if (fscanf(file, "BG %hhu %hhu %hhu\n", &scene->background_color.r, &scene->background_color.g, &scene->background_color.b) != 3)
    {
        printf("Error reading background color\n");
        fclose(file);
        return 1;
    }

    // Lettura del numero di sfere
    if (fscanf(file, "OBJ_N %d\n", &scene->num_spheres) != 1)
    {
        printf("Error reading number of spheres\n");
        fclose(file);
        return 1;
    }
    
    // Allocazione della memoria per le sfere
    scene->spheres = (Sphere *)malloc(scene->num_spheres * sizeof(Sphere));
    if (scene->spheres == NULL)
    {
        printf("Can't allocate memory for spheres\n");
        fclose(file);
        return 1;
    }

    // Lettura delle sfere
    for (int i = 0; i < scene->num_spheres; i++)
    {
       if (fscanf(file, "S %f %f %f %f %hhu %hhu %hhu %d %f\n", &scene->spheres[i].center.x,
         &scene->spheres[i].center.y, 
         &scene->spheres[i].center.z,
          &scene->spheres[i].radius, 
          &scene->spheres[i].color.r, 
          &scene->spheres[i].color.g, 
          &scene->spheres[i].color.b, 
          &scene->spheres[i].material, 
          &scene->spheres[i].mat_param) != 9)
       {
           printf("Error reading sphere\n");
           fclose(file);
           return 1;
       }
    }
    
    fclose(file); 
    return 0;
}

void setup_camera(Scene* scene, Vec3 origin, Vec3 lookat, double vfov, double aspect_ratio) {
    double theta = vfov * M_PI / 180.0;
    double h = tan(theta / 2.0);
    double viewport_height = 2.0 * h;
    double viewport_width = aspect_ratio * viewport_height;
    
    Vec3 w = normalize(sub(origin, lookat));
    
    // Vettore "up" di default
    Vec3 vup = {0, 1, 0};
    
    //caso camera parallela all'asse Y
    double dot_y = fabs(w.y);
    if (dot_y > 0.999) {
        vup = (Vec3){0, 0, 1};
    }
    
    Vec3 u = normalize(cross(w, vup)); 
    Vec3 v = cross(w, u);
    
    scene->camera.origin = origin;
    scene->camera.horizontal = mul_scalar(u, viewport_width);
    scene->camera.vertical = mul_scalar(v, viewport_height);
    scene->camera.lower_left_corner = sub(sub(sub(origin, 
                                              mul_scalar(u, viewport_width/2.0)), 
                                              mul_scalar(v, viewport_height/2.0)), 
                                          w);
}

void delete_scene(Scene *s)
{
    if (s == NULL) return;  
    free(s->spheres);
    free(s);
}