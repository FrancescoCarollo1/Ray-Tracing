// Francesco Carollo SM3201419

#include "render.h"
#include "vec3.h"
#include "color.h"
#include "ppm.h"
#include "scene.h"
#include <math.h>
#include <omp.h>
#include <stdint.h>
#include <stdbool.h>

uint32_t xorshift32(uint32_t *state)
{
    uint32_t x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
}

double random_double_fast(uint32_t *state)
{
    return (double)(xorshift32(state)) / 4294967295.0;
}

double random_double_range(uint32_t *state, double min, double max)
{
    return min + (max - min) * random_double_fast(state);
}

// Genera un vettore casuale dentro una sfera unitaria
Vec3 random_in_unit_sphere(uint32_t *state)
{
    while (1)
    {
        Vec3 p;
        p.x = random_double_range(state, -1, 1);
        p.y = random_double_range(state, -1, 1);
        p.z = random_double_range(state, -1, 1);
        if (prodotto_scalare(p, p) < 1)
            return p;
    }
}

Vec3 random_unit_vector(uint32_t *state)
{
    return normalize(random_in_unit_sphere(state));
}
// ---------------------------------------

Vec3 color_to_vec3(Color c)
{
    return (Vec3){(double)c.r / 255.0, (double)c.g / 255.0, (double)c.b / 255.0};
}

// Moltiplica due vettori componente per componente 
Vec3 mul_vec3(Vec3 v1, Vec3 v2)
{
    return (Vec3){v1.x * v2.x, v1.y * v2.y, v1.z * v2.z};
}

Vec3 lerp(Vec3 start, Vec3 end, double t)
{
    return add(mul_scalar(start, 1.0 - t), mul_scalar(end, t));
}


Vec3 reflect(Vec3 v, Vec3 n)
{
   
    double dot_vn = 2.0 * prodotto_scalare(v, n);

   
    return sub(v, mul_scalar(n, dot_vn));
}

Vec3 refract(Vec3 uv, Vec3 n, double etai_over_etat)
{
    double cos_theta = fmin(prodotto_scalare(mul_scalar(uv, -1.0), n), 1.0);
    Vec3 r_out_perp = mul_scalar(add(uv, mul_scalar(n, cos_theta)), etai_over_etat);
    Vec3 r_out_parallel = mul_scalar(n, -sqrt(fabs(1.0 - prodotto_scalare(r_out_perp, r_out_perp))));
    return add(r_out_perp, r_out_parallel);
}

double reflectance(double cosine, double ref_idx)
{
    // approssimazione di Schlick per la riflettanza
    double r0 = (1 - ref_idx) / (1 + ref_idx);
    r0 = r0 * r0;
    return r0 + (1 - r0) * pow((1 - cosine), 5);
}

float distanza_sfera(Vec3 ray_dir, Vec3 ray_origin, Sphere sphere)
{
    // ray = origin + t * dir
    // l'origine del raggio non è sempre (0,0,0)
    Vec3 oc = sub(ray_origin, sphere.center);
    float a = prodotto_scalare(ray_dir, ray_dir);
    float b = 2.0 * prodotto_scalare(oc, ray_dir);
    float c = prodotto_scalare(oc, oc) - sphere.radius * sphere.radius;
    float delta = (b * b) - (4 * a * c);

    if (delta < 0)
        return INFINITY;

    float t1 = (-b - sqrt(delta)) / (2.0 * a); // Usiamo -sqrt per trovare il più vicino
    if (t1 > 0.001)
        return t1;

    float t2 = (-b + sqrt(delta)) / (2.0 * a);
    if (t2 > 0.001)
        return t2;

    return INFINITY;
}

// --- FUNZIONE RICORSIVA ---
// depth: quanti rimbalzi ci rimangono
Vec3 colore_raggio_ricorsivo(Vec3 ray_origin, Vec3 ray_dir, Scene *scene, int depth, uint32_t *seed)
{
    // Caso base: se abbiamo finito i rimbalzi, ritorna nero (nessuna luce)
    if (depth <= 0)
    {
        return (Vec3){0.0, 0.0, 0.0};
    }

    float closest_t = INFINITY;
    int hit_index = -1;

    // Trova l'oggetto più vicino colpito
    for (int i = 0; i < scene->num_spheres; i++)
    {
        float t = distanza_sfera(ray_dir, ray_origin, scene->spheres[i]);
        if (t < closest_t)
        {
            closest_t = t;
            hit_index = i;
        }
    }

    // SE COLPISCE QUALCOSA
    if (hit_index != -1)
    {
        Sphere obj = scene->spheres[hit_index];

        // 1. Calcola il punto di impatto (P)
        Vec3 P = add(ray_origin, mul_scalar(ray_dir, closest_t));

        // 2. Calcola la Normale (N) nel punto di impatto
        Vec3 N = normalize(sub(P, obj.center));

        // 3. Determina se il raggio sta entrando o uscendo dalla superficie
        bool front_face = prodotto_scalare(ray_dir, N) < 0;
        Vec3 outward_normal = front_face ? N : mul_scalar(N, -1.0);

        // 4. Ottieni il colore dell'oggetto 
        Vec3 attenuation = color_to_vec3(obj.color);

        Vec3 scattered_dir;
        Vec3 scattered_origin;


        // --- Caso 1: LAMBERTIAN (Opaco/Diffuso) ---
        if (obj.material == 0)    
        {
            // Diffusione Lambertiana
            Vec3 rnd = random_unit_vector(seed);
            scattered_dir = normalize(add(outward_normal, rnd));
            
            // Offset nella direzione della normale 
            scattered_origin = add(P, mul_scalar(outward_normal, 0.001));
        }
        // --- Caso 2: METALLO (Specularità) ---
        else if (obj.material == 1)
        {
            // Riflessione perfetta (Law of Reflection)
            Vec3 reflected = reflect(normalize(ray_dir), outward_normal);

            // obj.mat_param è il valore di fuzziness (tra 0.0 e 1.0)
            scattered_dir = normalize(add(reflected, mul_scalar(random_in_unit_sphere(seed), obj.mat_param)));

            if (prodotto_scalare(scattered_dir, outward_normal) <= 0)
            {
                return (Vec3){0, 0, 0};
            }
            
            // Offset nella direzione della normale 
            scattered_origin = add(P, mul_scalar(outward_normal, 0.001));
        }
        // --- Caso 3: VETRO/DIELECTRIC (Rifrazione) ---
        else if (obj.material == 2)
        {
           
            attenuation = (Vec3){1.0, 1.0, 1.0};

            // Rapporto degli indici di rifrazione
            double refraction_ratio = front_face ? (1.0 / obj.mat_param) : obj.mat_param;

            Vec3 unit_ray_dir = normalize(ray_dir);
            
            // Calcola l'angolo (coseno e seno)
            double cos_theta = fmin(-prodotto_scalare(unit_ray_dir, outward_normal), 1.0);
            double sin_theta = sqrt(1.0 - cos_theta * cos_theta);

        
            bool cannot_refract = refraction_ratio * sin_theta > 1.0;

  
            double reflect_prob = reflectance(cos_theta, refraction_ratio);


            if (cannot_refract || reflect_prob > random_double_fast(seed))
            {
                // Riflessione
                scattered_dir = reflect(unit_ray_dir, outward_normal);
            }
            else
            {
                // Rifrazione
                scattered_dir = refract(unit_ray_dir, outward_normal, refraction_ratio);
            }

            // Per il vetro, l'offset dipende dalla direzione del raggio scattered
            // Se il raggio va verso l'interno, offset verso l'interno
            // Se il raggio va verso l'esterno, offset verso l'esterno
            double offset_direction = prodotto_scalare(scattered_dir, N) > 0 ? 1.0 : -1.0;
            scattered_origin = add(P, mul_scalar(N, offset_direction * 0.001));
        }
       
        else
        {
            // Se non è né 0, 1, né 2, assorbe la luce.
            return (Vec3){0, 0, 0};
        }

        // 5. Mescola il colore dell'oggetto con la luce che arriva dal rimbalzo
        return mul_vec3(attenuation, colore_raggio_ricorsivo(scattered_origin, scattered_dir, scene, depth - 1, seed));
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

                // Chiamata RICORSIVA
                Vec3 sample_color = colore_raggio_ricorsivo(cam.origin, ray_direction, scene, MAX_DEPTH, &seed);

                total_color_vec.x += sample_color.x;
                total_color_vec.y += sample_color.y;
                total_color_vec.z += sample_color.z;
            }

            // Media dei campioni
            double scale = 1.0 / SAMPLES_PER_PIXEL;
            total_color_vec.x *= scale;
            total_color_vec.y *= scale;
            total_color_vec.z *= scale;

            // Correzione gamma (gamma = 2.0)
            total_color_vec.x = sqrt(total_color_vec.x);
            total_color_vec.y = sqrt(total_color_vec.y);
            total_color_vec.z = sqrt(total_color_vec.z);

            // Conversione a colore finale
            Color final_color;
            final_color.r = (uint8_t)(fmin(total_color_vec.x, 1.0) * 255.999);
            final_color.g = (uint8_t)(fmin(total_color_vec.y, 1.0) * 255.999);
            final_color.b = (uint8_t)(fmin(total_color_vec.z, 1.0) * 255.999);

            pixel_out[i + j * width] = final_color;
        }
    }
}