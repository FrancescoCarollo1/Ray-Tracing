
#pragma once


typedef struct
{
    float x;
    float y;
    float z;
} Vec3;


Vec3 normalize(Vec3 v);

float prodotto_scalare(Vec3 v1, Vec3 v2);