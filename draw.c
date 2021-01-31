#include "math.h"
#include "x.h"

void draw() {
	clear_window();
	Vector3d rect[4] = {
		{1,1,1},{1,-1,1},{1,-1,3},{1,1,3}
	};
	Vector2d new_rect[4];

	for (int i = 0; i < 4; i++) {
		new_rect[i].i = (rect[i].i/rect[i].k)*100 + 200;
		new_rect[i].j = (rect[i].j/rect[i].k)*100 + 200;
	}

	for (int i = 0; i < 4; i++) {
		draw_line(new_rect[i], new_rect[(i+1)%4]);
	}
}
