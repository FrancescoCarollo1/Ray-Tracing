// Francesco Carollo SM3201419

// Questo file contiene le funzioni per la gestione dei vettori

#include "vec3.h"
#include <math.h>


Vec3 normalize(Vec3 v)
{
    float norm = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    Vec3 normalized = {v.x / norm, v.y / norm, v.z / norm};
    return normalized;
}

float prodotto_scalare(Vec3 v1, Vec3 v2)
{
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}
