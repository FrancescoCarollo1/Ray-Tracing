// Francesco Carollo SM3201419

#pragma once
#include "scene.h"
#include "color.h"

// Dichiarazione delle funzioni
void omp_render_scene(Scene *scene, Color *pixel_out, int width, int height, int samples_per_pixel, int frame_index);
Color colore_raggio(Vec3 ray, Scene *scene);
