//Francesco Carollo SM3201419

#pragma once
#include "scene.h"


//Define the function prototypes
void omp_render_scene (Scene *scene, Color *pixel_out, int width, int height);
void render_scene (Scene *scene, Color *pixel_out, int width, int height);
