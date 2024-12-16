#include "render.h"
#include "vec3.h"
#include <stdlib.h>
#include <math.h>
#include "ppm.h"






float distanza_sfera(Vec3 ray, Sphere sphere)
{
    float a = prodotto_scalare(ray, ray);
    float b = -2 * (prodotto_scalare(ray, sphere.center));
    float c = prodotto_scalare(sphere.center, sphere.center) - sphere.radius * sphere.radius;
    float delta = b * b - 4 * a * c;
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

void render_scene(Scene *scene, unsigned char *pixel_out, int width, int height)
{
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        { 
            Vec3 ray;
            ray.x = scene->viewport.width * (2 * i / (float)width - 1);
            ray.y = -scene->viewport.height * (2 * j / (float)height - 1);
            ray.z = scene->viewport.depth;
            Vec3 norm_ray = normalize(ray);
            Color pixel = colore_raggio(norm_ray, scene);
            pixel_out[0 + (i + j * width) * 3 ] = pixel.r;
            pixel_out[1 + (i + j * width) * 3 ] = pixel.g;
            pixel_out[2 + (i + j * width) * 3 ] = pixel.b;
        }
    }
}
