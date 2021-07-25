#include "math.h"
#include "player.h"

Player camera = {
	{0,0,0},
	{0,-M_PI_4,0}
};

void moveCamera(double magnitude) {
	int i;
	for (i = 0; i < DIR_CNT; i++) {
		if (camera.move_keys & (1 << i)) {
			//dies
		}
	}
}

void rotateCamera(double magnitude) {
	int i;
	for (i = 0; i < DIR_CNT; i++) {
		if (camera.rot_keys & (1 << i)) {
			if (i == LEFT || i == UP)
				camera.angle[(i/2)] += 0.001;
			else
				camera.angle[(i/2)] -= 0.001;
		}
	}
}
