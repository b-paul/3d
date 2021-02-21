#pragma once

#include "math.h"

typedef struct {
    Vector3d n;
    double d;
} Plane;

typedef Plane Frustum[6];
