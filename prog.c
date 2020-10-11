#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <xcb/xcb.h>

#define max(a, b) ((a) > (b) ? (a) : (b))

#define ROTATION_MASK 2
#define PI 3.14159265

typedef enum {
    DOWN, UP,
    LEFT, RIGHT,
    DIR_CNT
} Direction;

typedef struct {
    float x;
    float y;
} Point_2D;

typedef struct {
    float x;
    float y;
    float z;
} Point_3D;

typedef struct {
    /* ap, b and c values for the plane */
    Point_3D n;
    /* d value for the plane used for offset */
    float d;
    /* 
     * Boolean to determine whether a point is wanted
     * to be on the greater than side or less than side 
     */
    bool s;
} Plane;

/* Planes used for the frustum */
Plane frustum[5] = {
    {{0, 1, 0}, 10, true}
}

typedef struct {
    float floor_height;
    float ceiling_height;
    Point_2D points[2];
} Wall;

typedef struct {
    float x;
    float z;
    float yaw;
} Player;

/* List of walls */
Wall walls[] = {
    {0, 200, {{100, 100}, {100, 200}}}
};
int wall_cnt = 1;

/* Player variable using the above structure */
Player player = {50, 50, 0};

/* Mask used to store all currently pressed movement keys */
uint8_t movement_mask = 0;

/* Point used to represent the player in the 2d map */
xcb_point_t player_point;

/* Lane used to represent the direction the player is facing */
xcb_point_t player_line[2];

/* Large rectangle that is used to fill the screen, similar to clearing it */
xcb_rectangle_t clear_rectangle[] = {
    {0, 0, 10000, 10000}
};

/* Constants used for movement */
float movement_constants[DIR_CNT] = {
    2.0, -2.0, -(1.0/128.0)*PI, (1.0/128.0)*PI
};

/* Sleep for n milliseconds */
void
msleep(int n) {

    struct timespec ts;
    int res;

    ts.tv_sec = n/1000;
    ts.tv_nsec = (n % 1000) * 1000000;
    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);
}

void
draw(xcb_connection_t *connection, xcb_drawable_t window, xcb_gcontext_t foreground, xcb_screen_t *screen, int type) {
    /* clear the screen */
    uint32_t mask   = XCB_GC_FOREGROUND;
    uint32_t value = screen->black_pixel;
    xcb_change_gc(connection, foreground, mask, &value);

    xcb_poly_fill_rectangle(connection, window, foreground, 1, clear_rectangle);

    value = screen->white_pixel;
    xcb_change_gc(connection, foreground, mask, &value);

    /* Draw a 2D map showing the player and all the walls */
    if (type == 0) {
        /* 
         * Calculate the graphical coordinates of the player
         * based on the player struct
         */
        player_point.x = (int)player.x;
        player_point.y = (int)player.z;

        player_line[0].x = player_point.x + cos(player.yaw)*-5;
        player_line[0].y = player_point.y + sin(player.yaw)*-5;
        player_line[1].x = cos(player.yaw)*-15;
        player_line[1].y = sin(player.yaw)*-15;

        /* Draw the player to the screen */
        xcb_poly_point(connection, XCB_COORD_MODE_PREVIOUS, window, foreground, 1, &player_point);
        xcb_poly_line(connection, XCB_COORD_MODE_PREVIOUS, window, foreground, 2, player_line);

        /* Draw the walls to the screen */
        for (int i = 0; i < wall_cnt; i++) {
            xcb_point_t wall_line[] = {
                {walls[i].points[0].x, walls[i].points[0].y},
                {walls[i].points[1].x-walls[i].points[0].x, walls[i].points[1].y-walls[i].points[0].y},
            };

            xcb_poly_line(connection, XCB_COORD_MODE_PREVIOUS, window, foreground, 2, wall_line);
        }
    } 
    /* 
     * Draw a 2D map that draws the player in 
     * the middle and the walls relative to the player
     */
    else if (type == 1) {
        /* Set the player's location to the middle of the screen */
        player_point.x = 400;
        player_point.y = 500;

        player_line[0].x = 400;
        player_line[0].y = 495;
        player_line[1].x = 0;
        player_line[1].y = -10;

        /* Draw the player to the screen */
        xcb_poly_point(connection, XCB_COORD_MODE_PREVIOUS, window, foreground, 1, &player_point);
        xcb_poly_line(connection, XCB_COORD_MODE_PREVIOUS, window, foreground, 2, player_line);

        /* 
         * Find the location of the wall relative
         * to the player and draw it
         */
        for (int i = 0; i < wall_cnt; i++) {
            float wall_x1 = walls[i].points[0].x - player.x;
            float wall_x2 = walls[i].points[1].x - player.x;
            float wall_y1 = walls[i].points[0].y - player.z;
            float wall_y2 = walls[i].points[1].y - player.z;
            xcb_point_t wall_line[] = {
                {400 + wall_x1 * sin(player.yaw) - wall_y1 * cos(player.yaw), 500 + wall_x1 * cos(player.yaw) + wall_y1 * sin(player.yaw)},
                {400 + wall_x2 * sin(player.yaw) - wall_y2 * cos(player.yaw) - wall_line[0].x, 500 + wall_x2 * cos(player.yaw) + wall_y2 * sin(player.yaw) - wall_line[0].y},
            };

            xcb_poly_line(connection, XCB_COORD_MODE_PREVIOUS, window, foreground, 2, wall_line);
        }
    }
    /* Draw in 3D */
    else if (type == 2) {
        /* Draw the 3d representation of each wall */
        for (int i = 0; i < wall_cnt; i++) {
            float wall_x1 = walls[i].points[0].x - player.x;
            float wall_x2 = walls[i].points[1].x - player.x;
            float wall_y1 = walls[i].points[0].y - player.z;
            float wall_y2 = walls[i].points[1].y - player.z;

            float wall_z1 = wall_x1 * cos(player.yaw) + wall_y1 * sin(player.yaw);
            float wall_z2 = wall_x2 * cos(player.yaw) + wall_y2 * sin(player.yaw);
            wall_x1 = wall_x1 * sin(player.yaw) - wall_y1 * cos(player.yaw);
            wall_x2 = wall_x2 * sin(player.yaw) - wall_y2 * cos(player.yaw);

            if (wall_z1 >= 0 && wall_z2 >= 0)
                continue;
            if (wall_z1 >= 0 || wall_z2 >= 0) {
                float nearz = 1e-4f, farz = 5, nearside = 1e-5f, farside = 20;
            }

            xcb_point_t wall_points[] = {
                {400 - wall_x1 * 128 / wall_z1, 500 - (walls[i].floor_height - walls[i].ceiling_height) / wall_z1},
                {400 - wall_x2 * 128 / wall_z2, 500 - (walls[i].floor_height - walls[i].ceiling_height) / wall_z2},
                {400 - wall_x1 * 128 / wall_z1, 500 + (walls[i].floor_height - walls[i].ceiling_height) / wall_z1},
                {400 - wall_x2 * 128 / wall_z2, 500 + (walls[i].floor_height - walls[i].ceiling_height) / wall_z2},
            };

            xcb_point_t wall_line[] = {
                {0,0},{0,0}
            };

            wall_line[0] = wall_points[0];
            wall_line[1] = wall_points[1];
            wall_line[1].x -= wall_line[0].x;
            wall_line[1].y -= wall_line[0].y;
            xcb_poly_line(connection, XCB_COORD_MODE_PREVIOUS, window, foreground, 2, wall_line);
            wall_line[0] = wall_points[2];
            wall_line[1] = wall_points[3];
            wall_line[1].x -= wall_line[0].x;
            wall_line[1].y -= wall_line[0].y;
            xcb_poly_line(connection, XCB_COORD_MODE_PREVIOUS, window, foreground, 2, wall_line);
            wall_line[0] = wall_points[0];
            wall_line[1] = wall_points[2];
            wall_line[1].x -= wall_line[0].x;
            wall_line[1].y -= wall_line[0].y;
            xcb_poly_line(connection, XCB_COORD_MODE_PREVIOUS, window, foreground, 2, wall_line);
            wall_line[0] = wall_points[1];
            wall_line[1] = wall_points[3];
            wall_line[1].x -= wall_line[0].x;
            wall_line[1].y -= wall_line[0].y;
            xcb_poly_line(connection, XCB_COORD_MODE_PREVIOUS, window, foreground, 2, wall_line);
        }
    }

    xcb_flush(connection);
}

void
move(Direction dir) {
    /* If the direction is a rotation */
    if (dir & ROTATION_MASK) {
        player.yaw += movement_constants[dir];


        player.yaw = max(2*PI, player.yaw);
        player.yaw = max(2*PI, player.yaw+2*PI);

    /* 
     * The only other type of movement is
     * forward/backward movement
     */
    } else {
        player.x += movement_constants[dir] * cos(player.yaw);
        player.z += movement_constants[dir] * sin(player.yaw);
    }
}

int
main() {
    /* Flag determining whether to exit the program */
    bool exit = 0;

    /* Create a connection to the X server */
    xcb_connection_t *connection = xcb_connect(NULL, NULL);

    /* Get the screen data using the screen iterator */
    const xcb_setup_t *setup              = xcb_get_setup(connection);
    xcb_screen_iterator_t screen_iterator = xcb_setup_roots_iterator(setup);
    xcb_screen_t *screen                  = screen_iterator.data;

    /* 
     * Create a graphical environment using white as the
     * default colour when drawing
     */
    xcb_drawable_t window     = screen->root;
    xcb_gcontext_t foreground = xcb_generate_id(connection);
    uint32_t mask             = XCB_GC_FOREGROUND | XCB_GC_GRAPHICS_EXPOSURES;
    uint32_t values[2]        = {screen->white_pixel, 0};

    xcb_create_gc(connection, foreground, window, mask, values);


    window = xcb_generate_id(connection);

    mask      = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    values[0] = screen->black_pixel;
    values[1] = XCB_EVENT_MASK_EXPOSURE    | XCB_EVENT_MASK_KEY_PRESS
              | XCB_EVENT_MASK_KEY_RELEASE;

    xcb_create_window(connection,
                      XCB_COPY_FROM_PARENT,
                      window,
                      screen->root,
                      0, 0,
                      700, 700,
                      10,
                      XCB_WINDOW_CLASS_INPUT_OUTPUT,
                      screen->root_visual,
                      mask, values);

    xcb_map_window(connection, window);
    xcb_flush(connection);

    xcb_generic_event_t *event;

    int i, type = 1;
    while (!exit) {
        msleep(1000/60);
        draw(connection, window, foreground, screen, type);

        for (i = 0; i < DIR_CNT; i++) {
            if (movement_mask & (1 << i)) {
                move(i);
            }
        }

        while (event = xcb_poll_for_event(connection)) {
            switch (event->response_type & ~0x80) {
                case XCB_EXPOSE: {
                    draw(connection, window, foreground, screen, type);
                    break;
                }
                case XCB_KEY_PRESS: {
                    xcb_key_press_event_t *key_press_event = (xcb_key_press_event_t*) event;
                    /* W is 25, A is 38, S is 39, D is 40*/
                    switch(key_press_event->detail) {
                        case 25:
                            movement_mask |= 0x1 << UP;
                            break;
                        case 38:
                            movement_mask |= 0x1 << LEFT;
                            break;
                        case 39:
                            movement_mask |= 0x1 << DOWN;
                            break;
                        case 40:
                            movement_mask |= 0x1 << RIGHT;
                            break;
                        case 41:
                            type = 0;
                            break;
                        case 42:
                            type = 1;
                            break;
                        case 43:
                            type = 2;
                            break;
                    }


                    break;
                }
                case XCB_KEY_RELEASE: {
                    xcb_key_release_event_t *key_release_event = (xcb_key_release_event_t*) event;
                    switch(key_release_event->detail) {
                        case 25:
                            movement_mask &= ~(0x1 << UP);
                            break;
                        case 38:
                            movement_mask &= ~(0x1 << LEFT);
                            break;
                        case 39:
                            movement_mask &= ~(0x1 << DOWN);
                            break;
                        case 40:
                            movement_mask &= ~(0x1 << RIGHT);
                            break;
                    }
                    break;
                }
            }

            free(event);

        }
    }

    return 0;
}
