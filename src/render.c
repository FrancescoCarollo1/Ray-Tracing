// Francesco Carollo SM3201419

#include "render.h"
#include "vec3.h"
#include "color.h"
#include "ppm.h"
#include "scene.h"
#include <math.h>
#include <omp.h>
#include <stdint.h>

uint32_t xorshift32(uint32_t *state) {
    uint32_t x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
}

double random_double_fast(uint32_t *state) {
    return (double)(xorshift32(state)) / 4294967295.0;
}

double random_double_range(uint32_t *state, double min, double max) {
    return min + (max - min) * random_double_fast(state);
}

// Genera un vettore casuale dentro una sfera unitaria
Vec3 random_in_unit_sphere(uint32_t *state) {
    while (1) {
        Vec3 p;
        p.x = random_double_range(state, -1, 1);
        p.y = random_double_range(state, -1, 1);
        p.z = random_double_range(state, -1, 1);
        if (prodotto_scalare(p, p) < 1) return p;
    }
}

Vec3 random_unit_vector(uint32_t *state) {
    return normalize(random_in_unit_sphere(state));
}
// ---------------------------------------

Vec3 color_to_vec3(Color c) {
    return (Vec3){(double)c.r / 255.0, (double)c.g / 255.0, (double)c.b / 255.0};
}

// Moltiplica due vettori componente per componente (per mescolare i colori)
Vec3 mul_vec3(Vec3 v1, Vec3 v2) {
    return (Vec3){ v1.x * v2.x, v1.y * v2.y, v1.z * v2.z };
}

Vec3 lerp(Vec3 start, Vec3 end, double t) {
    return add(mul_scalar(start, 1.0 - t), mul_scalar(end, t));
}

float distanza_sfera(Vec3 ray_dir, Vec3 ray_origin, Sphere sphere) {
    // ray = origin + t * dir
    // Dobbiamo considerare l'origine del raggio ora, non è sempre (0,0,0)
    Vec3 oc = sub(ray_origin, sphere.center);
    float a = prodotto_scalare(ray_dir, ray_dir);
    float b = 2.0 * prodotto_scalare(oc, ray_dir);
    float c = prodotto_scalare(oc, oc) - sphere.radius * sphere.radius;
    float delta = (b * b) - (4 * a * c);
    
    if (delta < 0) return INFINITY;
    
    float t1 = (-b - sqrt(delta)) / (2.0 * a); // Usiamo -sqrt per trovare il più vicino
    if (t1 > 0.001) return t1; 
    
    float t2 = (-b + sqrt(delta)) / (2.0 * a);
    if (t2 > 0.001) return t2;
    
    return INFINITY;
}

// --- FUNZIONE RICORSIVA ---
// depth: quanti rimbalzi ci rimangono
Vec3 colore_raggio_ricorsivo(Vec3 ray_origin, Vec3 ray_dir, Scene *scene, int depth, uint32_t *seed)
{
    // Caso base: se abbiamo finito i rimbalzi, non c'è più luce
    if (depth <= 0) {
        return (Vec3){0, 0, 0};
    }

    float closest_t = INFINITY;
    int hit_index = -1;

    // Trova l'oggetto più vicino colpito
    for (int i = 0; i < scene->num_spheres; i++) {
        float t = distanza_sfera(ray_dir, ray_origin, scene->spheres[i]);
        if (t < closest_t) {
            closest_t = t;
            hit_index = i;
        }
    }

    // SE COLPISCE QUALCOSA
    if (hit_index != -1) {
        // 1. Calcola il punto di impatto (P)
        Vec3 P = add(ray_origin, mul_scalar(ray_dir, closest_t));
        
        // 2. Calcola la Normale (N) nel punto di impatto
        // (P - Center) normalizzato
        Vec3 N = normalize(sub(P, scene->spheres[hit_index].center));

        // 3. Calcola il bersaglio del rimbalzo (Target)
        // Target = P + N + RandomUnitVector (Lambertian distribution)
        Vec3 rnd = random_unit_vector(seed);
        Vec3 target = add(P, add(N, rnd));

        // 4. Calcola la nuova direzione del raggio
        Vec3 new_ray_dir = sub(target, P);

        // 5. Ottieni il colore dell'oggetto (attenuazione)
        Vec3 attenuation = color_to_vec3(scene->spheres[hit_index].color);

        // 6. RICORSIONE: Mescola il colore dell'oggetto con la luce che arriva dal rimbalzo
        // Colore = Attenuazione * Colore(nuovo_raggio)
        return mul_vec3(attenuation, colore_raggio_ricorsivo(P, new_ray_dir, scene, depth - 1, seed));
    }

    // SE NON COLPISCE NULLA
    Vec3 unit_direction = normalize(ray_dir);
    double t = 0.5 * (unit_direction.y + 1.0);
    Vec3 white = {1.0, 1.0, 1.0};
    Vec3 blue = {0.5, 0.7, 1.0};
    return lerp(white, blue, t);
}

void omp_render_scene(Scene *scene, Color *pixel_out, int width, int height)
{
    const int SAMPLES_PER_PIXEL = 50;
    const int MAX_DEPTH = 10; 

    Camera cam = scene->camera;

    #pragma omp parallel for collapse(2)
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            uint32_t seed = (i * 98765) ^ (j * 43210);
            Vec3 total_color_vec = {0.0, 0.0, 0.0};

            for (int s = 0; s < SAMPLES_PER_PIXEL; s++)
            {
                double u = ((double)i + random_double_fast(&seed)) / (width - 1);
                double v = ((double)j + random_double_fast(&seed)) / (height - 1);

                // Calcolo raggio
                Vec3 h_offset = mul_scalar(cam.horizontal, u);
                Vec3 v_offset = mul_scalar(cam.vertical, v);
                Vec3 target = add(cam.lower_left_corner, h_offset);
                target = add(target, v_offset);
                
                Vec3 ray_direction = sub(target, cam.origin);

                // Chiamata RICORSIVA invece che diretta
                // Passiamo 'cam.origin' come origine del raggio primario
                Vec3 sample_color = colore_raggio_ricorsivo(cam.origin, ray_direction, scene, MAX_DEPTH, &seed);

                total_color_vec.x += sample_color.x;
                total_color_vec.y += sample_color.y;
                total_color_vec.z += sample_color.z;
            }

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