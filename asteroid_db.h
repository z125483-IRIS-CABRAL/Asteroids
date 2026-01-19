#ifndef ASTEROID_DB_H
#define ASTEROID_DB_H

#include <stddef.h>

#define STR_MAX 128

typedef struct {
    char date[16];
    char name[STR_MAX];
    long id;
    int isHazardous;
    double absolute_magnitude_h;
    double diameter_min_m;
    double diameter_max_m;
    double miss_distance_km;
    double velocity_km_s;
} Asteroid;

typedef struct {
    Asteroid *data;
    size_t size;
    size_t cap;
} AsteroidDB;

#endif
