// Francesco Carollo SM3201419

#include "render.h"
#include "vec3.h"
#include <math.h>
#include <omp.h>
#include <stdlib.h>

// Questa funzione implementa il calcolo della distanza tra un raggio e una sfera
float distanza_sfera(Vec3 ray, Sphere sphere)
{
    float a = prodotto_scalare(ray, ray);
    float b = -2 * (prodotto_scalare(ray, sphere.center));
    float c = prodotto_scalare(sphere.center, sphere.center) - sphere.radius * sphere.radius;
    float delta = (b * b) - (4 * a * c);
    if (delta < 0)
    {
        return INFINITY;
    }
    
    float t1 = (-b + sqrt(delta)) / (2 * a);
    float t2 = (-b - sqrt(delta)) / (2 * a);
    if (t1 < 0 && t2 < 0)
    {
        return INFINITY;
    }
    if (t1 < 0)
    {
        return t2;
    }
    if (t2 < 0)
    {
        return t1;
    }
    return t1 < t2 ? t1 : t2;
}

// Questa funzione calcola il colore di un raggio
Color colore_raggio(Vec3 ray, Scene *scene)
{
    Color colore = scene->background_color;
    float distanza_minima = INFINITY;
    for (int i = 0; i < scene->num_spheres; i++)
    {
        float dist_s = distanza_sfera(ray, scene->spheres[i]);
        if (dist_s < distanza_minima)
        {
            distanza_minima = dist_s;
            colore = scene->spheres[i].color;
        }
    }
    return colore;
}

// Questa funzione riempie un array di pixel con i colori della scena
void omp_render_scene(Scene *scene, Color *pixel_out, int width, int height)
{
    #pragma omp parallel
    {
        #pragma omp for collapse(2)
        for (int i = 0; i < width; i++)
        {
            for (int j = 0; j < height; j++)
            {
                Vec3 ray;
                ray.x = (2 * i / (float)width - 1) * (scene->viewport.width / 2);
                ray.y = (2 * j / (float)height - 1) * (-scene->viewport.height / 2) ;
                ray.z = scene->viewport.depth ;
                Vec3 norm_ray = normalize(ray);
                pixel_out[i + j * width] = colore_raggio(norm_ray, scene);
            }
        }
    }
}
