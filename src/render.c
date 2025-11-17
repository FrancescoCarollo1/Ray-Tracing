// Francesco Carollo SM3201419

#include "render.h"
#include "vec3.h"
#include <math.h>
#include <omp.h>
#include <stdlib.h>

#include "color.h"  
#include "ppm.h"    
#include "scene.h"  


Vec3 color_to_vec3(Color c) {
    return (Vec3){(double)c.r / 255.0, (double)c.g / 255.0, (double)c.b / 255.0};
}

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


// Calcola il colore in formato Vec3 (float) per permettere la media.
Vec3 colore_raggio_vec3(Vec3 ray, Scene *scene)
{
    // Converte il colore di sfondo da Color (uint8_t) a Vec3 (float)
    Vec3 colore = color_to_vec3(scene->background_color);
    float distanza_minima = INFINITY;
    
    for (int i = 0; i < scene->num_spheres; i++)
    {
        float dist_s = distanza_sfera(ray, scene->spheres[i]);
        if (dist_s < distanza_minima)
        {
            distanza_minima = dist_s;
            // Converte il colore della sfera da Color (uint8_t) a Vec3 (float)
            colore = color_to_vec3(scene->spheres[i].color);
        }
    }
    return colore;
}


void omp_render_scene(Scene *scene, Color *pixel_out, int width, int height)
{
    
    const int SAMPLES_PER_PIXEL = 50;

    #pragma omp parallel
    {
        #pragma omp for collapse(2)
        for (int i = 0; i < width; i++)
        {
            for (int j = 0; j < height; j++)
            {
            
                Vec3 total_color_vec = {0.0, 0.0, 0.0};

                //Ciclo di campionamento (Antialiasing)
                for (int s = 0; s < SAMPLES_PER_PIXEL; s++)
                {
                    // Genera un offset casuale tra 0.0 e 1.0
                    double random_u = (double)rand() / (double)RAND_MAX;
                    double random_v = (double)rand() / (double)RAND_MAX;

                    double u = (double)i + random_u;
                    double v = (double)j + random_v;

                    // Crea il raggio 
                    Vec3 ray;
                    ray.x = (2 * u / (float)width - 1) * (scene->viewport.width / 2);
                    ray.y = (2 * v / (float)height - 1) * (-scene->viewport.height / 2) ;
                    ray.z = scene->viewport.depth ;
                    
                    Vec3 norm_ray = normalize(ray);

                    // Calcola il colore 
                    Vec3 sample_color = colore_raggio_vec3(norm_ray, scene);

                    // Accumula il colore 
                    total_color_vec.x += sample_color.x;
                    total_color_vec.y += sample_color.y;
                    total_color_vec.z += sample_color.z;
                }

                //media dei colori 
                double scale = 1.0 / SAMPLES_PER_PIXEL;
                total_color_vec.x *= scale;
                total_color_vec.y *= scale;
                total_color_vec.z *= scale;

                total_color_vec.x = sqrt(total_color_vec.x);
                total_color_vec.y = sqrt(total_color_vec.y);
                total_color_vec.z = sqrt(total_color_vec.z);


                Color final_color;
                final_color.r = (uint8_t)(total_color_vec.x * 255.999);
                final_color.g = (uint8_t)(total_color_vec.y * 255.999);
                final_color.b = (uint8_t)(total_color_vec.z * 255.999);
                pixel_out[i + j * width] = final_color;
            }
        }
    }
}