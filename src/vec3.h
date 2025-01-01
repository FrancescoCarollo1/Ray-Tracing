//Francesco Carollo SM3201419

#pragma once

//Define the data types

typedef struct
{
    float x;
    float y;
    float z;
} Vec3;

//Define the function prototypes

Vec3 normalize(Vec3 v);
float prodotto_scalare(Vec3 v1, Vec3 v2);