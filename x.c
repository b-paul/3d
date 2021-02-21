#include <xcb/xcb.h>

#include "draw.h"
#include "math.h"

extern double screen_size[2];

xcb_connection_t *connection;
xcb_screen_t *screen;
xcb_drawable_t window;
xcb_gcontext_t foreground;

void draw_line(Vector2d a, Vector2d b) {
    uint32_t mask   = XCB_GC_FOREGROUND;
    uint32_t value = screen->white_pixel;
    xcb_change_gc(connection, foreground, mask, &value);

	xcb_point_t x_line[2] = {
		{a[0], a[1]},
		{b[0], b[1]}
	};
	xcb_poly_line(connection, XCB_COORD_MODE_ORIGIN, window,
			foreground, 2, x_line);
	xcb_flush(connection);
}

void clear_window() {
    uint32_t mask   = XCB_GC_FOREGROUND;
    uint32_t value = screen->black_pixel;
    xcb_change_gc(connection, foreground, mask, &value);

	xcb_rectangle_t rect = {0,0,10000,10000};

	xcb_poly_fill_rectangle(connection, window, foreground, 1, &rect);
}

int
init_window() {
	/* Connect to the X server */
	connection = xcb_connect(NULL, NULL);
	if (xcb_connection_has_error(connection)) return 1;

	/* Get screen data with the screen iterator */
	const xcb_setup_t *setup = xcb_get_setup(connection);
	xcb_screen_iterator_t iterator = xcb_setup_roots_iterator(setup);
	screen = iterator.data;

	/* Create a graphical environment */
	window = screen->root;
	foreground = xcb_generate_id(connection);

	/* Create a graphics context */
	uint32_t mask = XCB_GC_FOREGROUND | XCB_GC_GRAPHICS_EXPOSURES;
	uint32_t values[2] = {screen->white_pixel, 0};
	xcb_create_gc(connection, foreground, window, mask, values);

	/* Allocate the Xid into window */
	window = xcb_generate_id(connection);

	/* Create the window with a black background */
	mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
	values[0] = screen->black_pixel;
    values[1] = XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS
		| XCB_EVENT_MASK_KEY_RELEASE;
	xcb_create_window(connection, XCB_COPY_FROM_PARENT, window, screen->root,
			0, 0, 700, 700, 10, XCB_WINDOW_CLASS_INPUT_OUTPUT,
			screen->root_visual, mask, values);

	/* Make the window visible and flush */
	xcb_map_window(connection, window);
	xcb_flush(connection);

    xcb_get_geometry_reply_t *geom;

    if ((geom = xcb_get_geometry_reply(connection, 
                    xcb_get_geometry(connection, window), NULL))) {
        screen_size[0] = geom->width;
        screen_size[1] = geom->height;
    }

	return 0;
}

void
parse_events() {
    xcb_generic_event_t *event;

	if ((event = xcb_poll_for_event(connection))) {
            switch (event->response_type & ~0x80) {
                case XCB_EXPOSE:
					draw();
					break;
			}
	}
}
