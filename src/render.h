// Francesco Carollo SM3201419

#pragma once
#include "scene.h"
#include "color.h"

// Dichiarazione delle funzioni
void omp_render_scene (Scene *scene, Color *pixel_out, int width, int height);
Color colore_raggio(Vec3 ray, Scene *scene);
