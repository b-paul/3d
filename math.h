#pragma once

#include <math.h>

typedef double Vector3d[3];

typedef double Vector2d[2];

double dot_product(Vector3d a, Vector3d b);
void cross_product(Vector3d a, Vector3d b, Vector3d o);
double magnitude(Vector3d v);
void unit_vector(Vector3d v, Vector3d o);
