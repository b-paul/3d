#include "draw.h"
#include "math.h"
#include "player.h"
#include "x.h"

double screen_size[2];

extern Player camera;

/* Interpolate between a and b so that z = z into vector o */
void interp_projection(double z, Vector3d a, Vector3d b, Vector3d o) {
	if (a[0] != b[0]) {
		if (a[1] != b[1]) {
			/* 
			 * The working out for this is in my math book 
			 * Basically m1 is slope for x, m2 is slope for y, and g is an
			 * offset value like the c in y=mx+c idk what the proper name is
			 */
			double m1, m2, g;
			m1 = (a[2] - b[2])/(a[0] - b[0]);
			m2 = (a[2] - b[2])/(a[1] - b[1]);
			g  = a[2]  - m2*a[1];

			o[0] = (z-g)/m1;
			o[1] = (z-g)/m2;
			o[2] = z;
		} else {
			/* y is constant so do the same above but only with 2 variables */
			o[1] = a[1];

			double m, g;
			m = (a[2] - b[2])/(a[0] - b[0]);
			g = a[2] - m*a[0];

			o[0] = (z-g)/m;
			o[2] = z;
		}
	} else {
		if (a[1] != b[1]) {
			/* x is constant so do the same above but only with 2 variables */
			o[0] = a[0];

			double m, g;
			m = (a[2] - b[2])/(a[1] - b[1]);
			g = a[2] - m*a[1];

			o[1] = (z-g)/m;
			o[2] = z;
		} else {
			/*
			 * both x and y are constant so the equation would be like x=5 y=2
			 * or something so you dont need to calculate anything
			 */
			o[0] = a[0];
			o[1] = a[1];
			o[2] = z;
		}
	}
}

void project(Vector3d i, Vector2d o) {
	o[0] = (i[0]/i[2])*screen_size[0]/2 + screen_size[0]/2;
	o[1] = (i[1]/i[2])*screen_size[1]/2 + screen_size[1]/2;
}

void draw() {
	int i,j;

	clear_window();

	Vector3d rect_p[4] = {
		{2,1,-5},{2,-1,-5},{2,-1,4},{2,1,4}
	};

	/* translate and rotate based on player position */
	for (i = 0; i < 4; i++) {
		double x,y,z,theta;

		x = rect_p[i][0];
		y = rect_p[i][1];
		theta = camera.angle[2];
		rect_p[i][0] = x*cos(theta) - y*sin(theta);
		rect_p[i][1] = x*sin(theta) + y*cos(theta);

		x = rect_p[i][0];
		z = rect_p[i][2];
		theta = camera.angle[1];
		rect_p[i][0] = x*cos(theta) + z*sin(theta);
		rect_p[i][2] = -x*sin(theta) + z*cos(theta);

		y = rect_p[i][1];
		z = rect_p[i][2];
		theta = camera.angle[0];
		rect_p[i][1] = y*cos(theta) - z*sin(theta);
		rect_p[i][2] = y*sin(theta) + z*cos(theta);
	}

	Line rect_l[4] = {
		{ &rect_p[0], &rect_p[1] },
		{ &rect_p[1], &rect_p[2] },
		{ &rect_p[2], &rect_p[3] },
		{ &rect_p[3], &rect_p[0] }
	};

	for (i = 0; i < 4; i++) {
		if ((*rect_l[i][0])[2] < 1) {
			/*
			 * If p1 is behind the camera interpolate so weird stuff doesn't
			 * happen
			 */
			if ((*rect_l[i][1])[2] >= 1) {
				Vector3d new_p1;
				interp_projection(1, *rect_l[i][0], *rect_l[i][1], new_p1);

				Vector2d p1, p2;
				project(new_p1, p1);
				project(*rect_l[i][1], p2);

				draw_line(p1, p2);
			}
		} else {
			/* Same but with p2 */
			if ((*rect_l[i][1])[2] < 1) {
				Vector3d new_p2;
				interp_projection(1, *rect_l[i][0], *rect_l[i][1], new_p2);

				Vector2d p1, p2;
				project(*rect_l[i][0], p1);
				project(new_p2, p2);

				draw_line(p1, p2);
			} else {
				Vector2d p1, p2;
				project(*rect_l[i][0], p1);
				project(*rect_l[i][1], p2);
				draw_line(p1, p2);
			}
		}
	}
}
