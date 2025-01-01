#include "test.h"
#include <omp.h>


// Queste funzioni prendono in input un puntatore a funzione e restituiscono il tempo impiegato per eseguire la funzione passata come argomento

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