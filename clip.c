#include <stdbool.h>

#include "clip.h"
#include "math.h"

Frustum clip_frustum;

void create_clip_fustum(double width, double height, double near, double far) {
	clip_frustum[0] = (Plane) {{0, 0, 1}, -near};
	clip_frustum[1] = (Plane) {{0, 0, -1},  far};

	Vector3d corners[4] = {
		{-(width/2.0), (height/2.0), 10},
		{(width/2.0), (height/2.0), 10},
		{(width/2.0), -(height/2.0), 10},
		{-(width/2.0), -(height/2.0), 10}
	};

	for (int i = 2; i < 6; i++) {
		Vector3d cross, n;
		cross_product(corners[i%4], corners[(i+1)%4], cross);
		unit_vector(cross, n);
		clip_frustum[i] = (Plane) {*n, 0};
	}
}

bool point_clips_frustum(Vector3d point) {
	for (int i = 0; i < 6; i++) {
		if (dot_product(point, clip_frustum[i].n) + clip_frustum[i].d < 0)
			return false;
	}
	return true;
}
