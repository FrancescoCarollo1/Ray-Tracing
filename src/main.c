// Francesco Carollo SM3201419

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "ppm.h"
#include "scene.h"
#include "render.h"
#include <string.h>
#include <omp.h>



double test_render_scene(void (* render_scene)(Scene *, Color *, int, int))
{
    int width = 1920;
    int height = 1080;
    Color *data = malloc(width * height * 3);
    memset(data, 0, width * height * 3);

    Scene *scene = malloc(sizeof(Scene));
    read_scene("prova.txt", scene);

    double start = omp_get_wtime();
    render_scene(scene, data, width, height);
    double end = omp_get_wtime();
    double ms = (end - start) * 1000;
    return ms;
    printf("Time: %f ms\n", ms);
    return 0;
}

int main()
{
    int width = 1920;
    int height = 1080;
    Color *data = malloc(width * height * 3);
    memset(data, 0, width * height * 3);
    
    Scene *scene = malloc(sizeof(Scene));
    read_scene("prova.txt", scene);
    render_scene(scene, data, width, height);

    printf ("Time: tempo per renderizzare %f ms\n", test_render_scene(render_scene));
    printf ("Time: tempo per renderizzare in parallelo %f ms\n", test_render_scene(omp_render_scene));        scrivi_immagine("prova.ppm", data, width, height);
    scrivi_immagine("prova.ppm", data, width, height);

}

