//Francesco Carollo SM3201419

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "ppm.h"


int main ()
{
    unsigned char data[640*480*3];
    for (int i = 0; i < 640*480*3; i++)
    {
        data[i] = i % 256;
    }

    scrivi_immagine("prova.ppm", data, 640, 480);
}
