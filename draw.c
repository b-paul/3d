#include <xcb/xcb.h>

xcb_connection_t *connection;
xcb_screen_t *screen;
xcb_drawable_t window;
xcb_gcontext_t foreground;

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
	xcb_create_window(connection, XCB_COPY_FROM_PARENT, window, screen->root,
			0, 0, 700, 700, 10, XCB_WINDOW_CLASS_INPUT_OUTPUT,
			screen->root_visual, mask, values);

	/* Make the window visible and flush */
	xcb_map_window(connection, window);
	xcb_flush(connection);

	return 0;
}
