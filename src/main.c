// Francesco Carollo SM3201419

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "ppm.h"
#include "scene.h"
#include "render.h"
#include <string.h>
#include <omp.h>




int main(int argc, char *argv[]) {
    if (argc != 5) {
        printf("Utilizzo corretto: %s scene_file.txt image_file.ppm width height\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *scene_file = argv[1];
    const char *image_file = argv[2];
    int width = atoi(argv[3]);
    int height = atoi(argv[4]);

    Color *pixel_data = malloc(width * height * sizeof(Color));
    memset(pixel_data, 0, width * height * sizeof(Color));
    Scene *scene = create_empty_scene();

    if ( read_scene(scene_file, scene) != 0) {
        printf("Errore nel caricamento della scena\n");
        exit(EXIT_FAILURE);
    }

    omp_render_scene(scene, pixel_data, width, height);
    scrivi_immagine(image_file, pixel_data, width, height);
    printf("Successo. Il rendering è stato salvato come %s\n", image_file);
    delete_scene(scene);

    exit(EXIT_SUCCESS);
}

