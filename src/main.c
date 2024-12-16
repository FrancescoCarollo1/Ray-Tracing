//Francesco Carollo SM3201419

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "ppm.h"
#include "scene.h"
#include "render.h"
#include <string.h>


int main ()
{
    int width = 1920;
    int height = 1080;
    unsigned char *data = malloc(width*height*3);
    memset (data, 0, width*height*3);
    Scene *scene = malloc(sizeof(Scene));
    read_scene("prova.txt", scene);
    render_scene(scene, data, width, height);
    scrivi_immagine("prova.ppm", data, width, height);
    return 0;
}   
