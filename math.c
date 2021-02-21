#include <math.h>

#include "math.h"

double dot_product(Vector3d a, Vector3d b) {
    return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}

void cross_product(Vector3d a, Vector3d b, Vector3d o) {
	o[0] = a[1]*b[2] - a[2]*b[1];
	o[1] = a[2]*b[0] - a[0]*b[2];
	o[2] = a[0]*b[1] - a[1]*b[0];
}

double magnitude(Vector3d v) {
	return sqrt(dot_product(v, v));
}

void unit_vector(Vector3d v, Vector3d o) {
	double m = magnitude(v);
	o[0] = v[0]/m;
	o[1] = v[1]/m;
	o[2] = v[2]/m;
}
