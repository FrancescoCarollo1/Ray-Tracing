// Francesco Carollo SM3201419

// Questo file contiene le funzioni per la gestione dei vettori

#include "vec3.h"
#include <math.h>


Vec3 normalize(Vec3 v) {
    double len = sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
    if (len < 1e-8) { 
        return (Vec3){0, 0, 0};
    }
    return (Vec3){v.x/len, v.y/len, v.z/len};
}

float prodotto_scalare(Vec3 v1, Vec3 v2)
{
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

Vec3 cross(Vec3 a, Vec3 b) {
    return (Vec3){
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

Vec3 sub(Vec3 v1, Vec3 v2)
{
    Vec3 result;
    result.x = v1.x - v2.x;
    result.y = v1.y - v2.y;
    result.z = v1.z - v2.z;
    return result;
}

Vec3 add(Vec3 v1, Vec3 v2)
{
    Vec3 result;
    result.x = v1.x + v2.x;
    result.y = v1.y + v2.y;
    result.z = v1.z + v2.z;
    return result;
}

Vec3 mul_scalar(Vec3 v, double t)
{
    Vec3 result;
    result.x = v.x * t;
    result.y = v.y * t;
    result.z = v.z * t;
    return result;
}