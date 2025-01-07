//Francesco Carollo SM3201419

#pragma once

// Definizione della struttura dati
typedef struct
{
    float x;
    float y;
    float z;
} Vec3;

//Prototipi delle funzioni
Vec3 normalize(Vec3 v);
float prodotto_scalare(Vec3 v1, Vec3 v2);