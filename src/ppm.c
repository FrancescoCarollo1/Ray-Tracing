// Francesco Carollo SM3201419

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stddef.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include "scene.h"

// Note: cleanup is done by the OS in case of error

// La funzione scrivi_immagine scrive un'immagine in formato PPM
int scrivi_immagine(const char *filename, Color *data, int width, int height)
{
    // Calculate data size
    // Calculate the number of digits needed for width and height
    int w = (int)((ceil(log10(width))) * sizeof(char));
    int h = (int)((ceil(log10(height))) * sizeof(char));
    int preambolo_size = 3 + w + 1 + h + 1 + 4 + 1;
    int data_size = width * height * 3;

    // Create file header
    char *preambolo = (char *)malloc((preambolo_size) * sizeof(char));
    if (preambolo == NULL)
    {
        perror("Can't allocate memory for preambolo");
        return 1;
    }
    sprintf(preambolo, "P6\n%d %d\n255\n", width, height);

    // Open file and get file descriptor
    FILE *file = fopen(filename, "w+");
    if (file == NULL)
    {
        perror("Error opening file");
        return 1;
    }

    int fd = fileno(file);
    if (fd == -1)
    {
        perror("Error getting file descriptor");
        return 1;
    }

    // Ensure file is large enough
    if (ftruncate(fd, preambolo_size + data_size) == -1)
    {
        perror("Error setting file size");
        return 1;
    }

    // Stat file needed for mmap
    struct stat statbuf;
    if (stat(filename, &statbuf) < 0)
    {
        perror("stat error");
        return 1;
    }

    void *addr = mmap(NULL, statbuf.st_size, PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED)
    {
        perror("Error mapping the file");
        return 1;
    }

    // Write data
    memcpy(addr, preambolo, preambolo_size);
    memcpy((char *)addr + preambolo_size - 1, data, data_size);

    // Cleanup
    free(preambolo);
    munmap(addr, statbuf.st_size);
    fclose(file);

    return 0;
}
