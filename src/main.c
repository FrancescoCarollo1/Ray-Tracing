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


double test_colore_raggio(Color (*colore_raggio)(Vec3 ray, Scene *scene))
{
    double start = omp_get_wtime();
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
    Color *data = malloc(width * height * sizeof(Color));
    memset(data, 0, width * height * sizeof(Color));
    Scene *scene = create_empty_scene();
    read_scene("prove_txt/prova11.txt", scene);
    render_scene(scene, data, width, height);
    scrivi_immagine("renders/prova11.ppm", data, width, height);

    free(data);
    delete_scene(scene);

}

