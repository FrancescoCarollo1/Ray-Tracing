// Francesco Carollo SM3201419

#pragma once

// Definizione della struttura dati
typedef struct
{
    float x;
    float y;
    float z;
} Vec3;

// Dichiarazione delle funzioni
Vec3 normalize(Vec3 v);
float prodotto_scalare(Vec3 v1, Vec3 v2);
Vec3 cross(Vec3 v1, Vec3 v2);
Vec3 sub(Vec3 v1, Vec3 v2);
Vec3 add(Vec3 v1, Vec3 v2);
Vec3 mul_scalar(Vec3 v, double t);