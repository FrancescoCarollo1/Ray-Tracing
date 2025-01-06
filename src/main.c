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
        printf("Usage: %s [scene_file.txt] [image_file.ppm] [width] [height]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    const char *scene_file = argv[1];
    const char *image_file = argv[2];
    int width = atoi(argv[3]);
    int height = atoi(argv[4]);
    Color *data = malloc(width * height * sizeof(Color));
    memset(data, 0, width * height * sizeof(Color));
    Scene *scene = create_empty_scene();
    read_scene("prove_txt/prova1.txt", scene);

    if (scene == NULL) {
        printf("Errore nel caricamento della scena\n");
        exit(EXIT_FAILURE);
    }

    render_scene(scene, data, width, height);
    scrivi_immagine("renders/prova1.ppm", data, width, height);
    printf("Successo. Il rendering è stato salvato come %s\n", image_file);
    free(scene);

    exit(EXIT_SUCCESS);
}

