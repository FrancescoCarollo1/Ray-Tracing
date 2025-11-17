// Francesco Carollo SM3201419

#include "render.h"
#include "vec3.h"
#include <math.h>
#include <omp.h>
#include <stdlib.h>

// --- MODIFICA 1: Aggiunti gli header necessari ---
#include "color.h"  // Per la struct Color (uint8_t)
#include "ppm.h"    // (Probabilmente definisce Color o Vec3)
#include "scene.h"  // Per la struct Scene

// --- MODIFICA 2: Funzione helper ---
// Converte la tua struct Color (uint8_t 0-255) in Vec3 (float 0.0-1.0)
// per i calcoli.
Vec3 color_to_vec3(Color c) {
    return (Vec3){(double)c.r / 255.0, (double)c.g / 255.0, (double)c.b / 255.0};
}

// Questa funzione implementa il calcolo della distanza tra un raggio e una sfera
// (Questa funzione è invariata)
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

// --- MODIFICA 3: La funzione 'colore_raggio' ora usa Vec3 ---
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

// --- MODIFICA 4: 'omp_render_scene' aggiornata per Antialiasing ---
// Questa funzione riempie un array di pixel (di tipo Color - uint8_t)
// con i colori della scena, implementando l'antialiasing.
void omp_render_scene(Scene *scene, Color *pixel_out, int width, int height)
{
    // Inizia con 50 campioni. Puoi aumentarlo a 100 o più
    // per una qualità migliore (ma un rendering più lento).
    const int SAMPLES_PER_PIXEL = 50;

    #pragma omp parallel
    {
        #pragma omp for collapse(2)
        for (int i = 0; i < width; i++)
        {
            for (int j = 0; j < height; j++)
            {
                // 1. Accumulatore in Vec3 (float) per i calcoli
                Vec3 total_color_vec = {0.0, 0.0, 0.0};

                // 2. Ciclo di campionamento (Antialiasing)
                for (int s = 0; s < SAMPLES_PER_PIXEL; s++)
                {
                    // Genera un offset casuale tra 0.0 e 1.0
                    double random_u = (double)rand() / (double)RAND_MAX;
                    double random_v = (double)rand() / (double)RAND_MAX;

                    double u = (double)i + random_u;
                    double v = (double)j + random_v;

                    // Crea il raggio (in formato Vec3)
                    Vec3 ray;
                    ray.x = (2 * u / (float)width - 1) * (scene->viewport.width / 2);
                    ray.y = (2 * v / (float)height - 1) * (-scene->viewport.height / 2) ;
                    ray.z = scene->viewport.depth ;
                    
                    Vec3 norm_ray = normalize(ray);

                    // Calcola il colore (in formato Vec3)
                    Vec3 sample_color = colore_raggio_vec3(norm_ray, scene);

                    // Accumula il colore (in formato Vec3)
                    total_color_vec.x += sample_color.x;
                    total_color_vec.y += sample_color.y;
                    total_color_vec.z += sample_color.z;
                }

                // 3. Fai la media dei colori (sempre in Vec3)
                double scale = 1.0 / SAMPLES_PER_PIXEL;
                total_color_vec.x *= scale;
                total_color_vec.y *= scale;
                total_color_vec.z *= scale;

                // 4. Correzione Gamma (sqrt) - Fondamentale
                total_color_vec.x = sqrt(total_color_vec.x);
                total_color_vec.y = sqrt(total_color_vec.y);
                total_color_vec.z = sqrt(total_color_vec.z);

                // --- 5. TRADUZIONE FINALE (da Vec3 a Color) ---
                // Converti il colore finale (float 0.0-1.0)
                // nel formato di output (uint8_t 0-255).
                Color final_color;
                final_color.r = (uint8_t)(total_color_vec.x * 255.999);
                final_color.g = (uint8_t)(total_color_vec.y * 255.999);
                final_color.b = (uint8_t)(total_color_vec.z * 255.999);

                // 6. Scrivi il colore finale (in formato uint8_t) nel buffer
                pixel_out[i + j * width] = final_color;
            }
        }
    }
}