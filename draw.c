#include "math.h"
#include "x.h"

double screen_size[2];

void draw() {
	clear_window();
	Vector3d rect[4] = {
		{1,1,1},{1,-1,1},{1,-1,3},{1,1,3}
	};
	Vector2d new_rect[4];

	for (int i = 0; i < 4; i++) {
		new_rect[i][0] = (rect[i][0]/rect[i][2])*100 + 200;
		new_rect[i][1] = (rect[i][1]/rect[i][2])*100 + 200;
	}

	for (int i = 0; i < 4; i++) {
		draw_line(new_rect[i], new_rect[(i+1)%4]);
	}
}
