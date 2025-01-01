// Francesco Carollo SM3201419

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "ppm.h"
#include "scene.h"
#include "render.h"
#include <string.h>
#include <omp.h>









int main()
{
    int width = 1920;
    int height = 1080;
    Color *data = malloc(width * height * sizeof(Color));
    memset(data, 0, width * height * sizeof(Color));
    Scene *scene = create_empty_scene();
    read_scene("prove_txt/prova8.txt", scene);
    render_scene(scene, data, width, height);
    scrivi_immagine("renders/prova8.ppm", data, width, height);

    free(data);
    delete_scene(scene);

}

