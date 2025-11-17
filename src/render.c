// Francesco Carollo SM3201419
// Questo file contiene le funzioni per il rendering della scena

#include "render.h"
#include "vec3.h"
#include "color.h"
#include "ppm.h"
#include "scene.h"
#include <math.h>
#include <omp.h>
#include <stdlib.h>
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
    const int SAMPLES_PER_PIXEL = 5;
    
    Camera cam = scene->camera;

    #pragma omp parallel
    {
        #pragma omp for collapse(2)
        for (int i = 0; i < width; i++)
        {
            for (int j = 0; j < height; j++)
            {
                Vec3 total_color_vec = {0.0, 0.0, 0.0};

                for (int s = 0; s < SAMPLES_PER_PIXEL; s++)
                {
                    // Calcola (u,v) normalizzati (0.0 a 1.0) con offset
                    double u = ((double)i + (double)rand() / (double)RAND_MAX) / (width - 1);
                    double v = ((double)j + (double)rand() / (double)RAND_MAX) / (height - 1);

                    // Calcola la direzione del raggio dalla camera
                    
                    Vec3 h_offset = mul_scalar(cam.horizontal, u);
                    Vec3 v_offset = mul_scalar(cam.vertical, v);
                    Vec3 target = add(cam.lower_left_corner, h_offset);
                    target = add(target, v_offset);
                    
                    Vec3 ray_direction = sub(target, cam.origin);
                    Vec3 norm_ray = normalize(ray_direction);

                    Vec3 sample_color = colore_raggio_vec3(norm_ray, scene);

                    // Accumula il colore
                    total_color_vec.x += sample_color.x;
                    total_color_vec.y += sample_color.y;
                    total_color_vec.z += sample_color.z;
                }

                // ... (media, correzione gamma, e traduzione finale in 'Color')
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