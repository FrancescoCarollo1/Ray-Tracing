// Francesco Carollo SM3201419

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

int scrivi_immagine(const char *filename, unsigned char *data, int width, int height)
{


    FILE *file = fopen(filename, "w");
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


    int w = (int)((ceil(log10(width))+1)*sizeof(char));
    int h = (int)((ceil(log10(height))+1)*sizeof(char)); 
    int preambolo_size = 3 + w + 1+ h + 5 + 1;
    char *preambolo = (char*) malloc((preambolo_size)*sizeof(char));
    sprintf(preambolo, "P6\n%d %d\n255\n", width, height);


    memcpy(addr, preambolo, preambolo_size);

    memcpy((char*)addr + preambolo_size, data, width * height * 3);
    free(preambolo);
    munmap(addr, statbuf.st_size);
    fclose(file);
    
    return 0;
}


