#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <xcb/xcb.h>

#define max(a, b) ((a) > (b) ? (a) : (b))

#define DIR_MASK 2
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
} Plane;

/* Planes used for the frustum */
Plane frustum[5] = {
    {{0, 1, 0}, 10}
};

typedef struct {
    Point_3D points[3];
} Triangle;

typedef struct {
    Point_3D pos;
    Point_3D angle;

    float fov;
} Player;

/* List of walls */
Triangle triangles[] = {
    {{{100, -100, 100}, {100, -50, 100}, {100, -50,  110}}},
    {{{100, -100, 100}, {100, -50, 110}, {100, -100, 110}}},
    {{{100, -100, 150}, {100, -50, 150}, {100, -50,  160}}},
    {{{100, -100, 150}, {100, -50, 160}, {100, -100, 160}}},
    {{{110, -100, 100}, {110, -50, 100}, {110, -50,  110}}},
    {{{110, -100, 100}, {110, -50, 110}, {110, -100, 110}}},
    {{{110, -100, 150}, {110, -50, 150}, {110, -50,  160}}},
    {{{110, -100, 150}, {110, -50, 160}, {110, -100, 160}}},
    {{{100, -100, 100}, {100, -50, 100}, {110, -50,  100}}},
    {{{100, -100, 100}, {110, -50, 100}, {110, -100, 100}}},
    {{{100, -100, 110}, {100, -50, 110}, {110, -50,  110}}},
    {{{100, -100, 110}, {110, -50, 110}, {110, -100, 110}}},
    {{{100, -100, 150}, {100, -50, 150}, {110, -50,  150}}},
    {{{100, -100, 150}, {110, -50, 150}, {110, -100, 150}}},
    {{{100, -100, 160}, {100, -50, 160}, {110, -50,  160}}},
    {{{100, -100, 160}, {110, -50, 160}, {110, -100, 160}}},
    {{{150, -100, 100}, {150, -50, 100}, {150, -50,  110}}},
    {{{150, -100, 100}, {150, -50, 110}, {150, -100, 110}}},
    {{{150, -100, 150}, {150, -50, 150}, {150, -50,  160}}},
    {{{150, -100, 150}, {150, -50, 160}, {150, -100, 160}}},
    {{{160, -100, 100}, {160, -50, 100}, {160, -50,  110}}},
    {{{160, -100, 100}, {160, -50, 110}, {160, -100, 110}}},
    {{{160, -100, 150}, {160, -50, 150}, {160, -50,  160}}},
    {{{160, -100, 150}, {160, -50, 160}, {160, -100, 160}}},
    {{{150, -100, 100}, {150, -50, 100}, {160, -50,  100}}},
    {{{150, -100, 100}, {160, -50, 100}, {160, -100, 100}}},
    {{{150, -100, 110}, {150, -50, 110}, {160, -50,  110}}},
    {{{150, -100, 110}, {160, -50, 110}, {160, -100, 110}}},
    {{{150, -100, 150}, {150, -50, 150}, {160, -50,  150}}},
    {{{150, -100, 150}, {160, -50, 150}, {160, -100, 150}}},
    {{{150, -100, 160}, {150, -50, 160}, {160, -50,  160}}},
    {{{150, -100, 160}, {160, -50, 160}, {160, -100, 160}}},
    {{{100, -50,  100}, {160, -50, 100}, {160, -50,  160}}},
    {{{100, -50,  100}, {160, -50, 160}, {100, -50,  160}}},
    {{{100, -50,  100}, {100, -40, 100}, {100, -40,  160}}},
    {{{100, -50,  100}, {100, -40, 160}, {100, -50,  160}}},
    {{{160, -50,  100}, {160, -40, 100}, {160, -40,  160}}},
    {{{160, -50,  100}, {160, -40, 160}, {160, -50,  160}}},
    {{{100, -50,  100}, {100, -40, 100}, {160, -40,  100}}},
    {{{100, -50,  100}, {160, -40, 100}, {160, -50,  100}}},
    {{{100, -50,  160}, {100, -40, 160}, {160, -40,  160}}},
    {{{100, -50,  160}, {160, -40, 160}, {160, -50,  160}}},
    {{{100, -40,  100}, {160, -40, 100}, {160, -40,  160}}},
    {{{100, -40,  100}, {160, -40, 160}, {100, -40,  160}}},
};
int triangle_cnt = 44;

/* Player variable using the above structure */
Player player = {{200, 0, 150}, {0, 0, 0}, PI/2};

/* Mask used to store all currently pressed movement keys */
uint8_t movement_mask = 0;

/* Mask used to stora all currently pressed rotation keys */
uint8_t rotation_mask = 0;

/* Screen size used for fov calculation etc */
uint16_t screen_size[2];

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
    2.0, -2.0, 2.0, -2.0
};

Point_2D rotation_constants[DIR_CNT] = {
    {-0.1, 0.0}, {0.1, 0.0}, {0.0, 0.1}, {0.0, -0.1}
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
    int i,j;

    /* clear the screen */
    uint32_t mask   = XCB_GC_FOREGROUND;
    uint32_t value = screen->black_pixel;
    xcb_change_gc(connection, foreground, mask, &value);

    xcb_poly_fill_rectangle(connection, window, foreground, 1, clear_rectangle);

    value = screen->white_pixel;
    xcb_change_gc(connection, foreground, mask, &value);

    /* Draw in 3D */

    /* 
     * Distance from player eye to left/right side of monitor with a
     * certain fov
     */
    float dist = (screen_size[0]*sin(PI-player.fov/2))/sin(player.fov);
    float display_z = (sqrt(4*dist*dist - screen_size[0]*screen_size[0]))/2;

    /* Draw the 3d representation of each triangle */
    for (i = 0; i < triangle_cnt; i++) {

        /* Translate all points on the triangle to be relative to the player */
        Point_3D translated_points[3];

        for (j = 0; j < 3; j++) {
            translated_points[j].x = triangles[i].points[j].x - player.pos.x;
            translated_points[j].y = -triangles[i].points[j].y - player.pos.y;
            translated_points[j].z = triangles[i].points[j].z - player.pos.z;
        }

        /* Rotate all the points on the triangle to be relative to the player */

        Point_3D points[3];

        for (j = 0; j < 3; j++) {
            /* Rotate on the y axis */
            points[j].x = cos(player.angle.y) * translated_points[j].x +
                          sin(player.angle.y) * translated_points[j].z;

            points[j].y = translated_points[j].y;

            points[j].z =-sin(player.angle.y) * translated_points[j].x +
                          cos(player.angle.y) * translated_points[j].z;

            /* Put these numbers into the translated_points variable for reuse */
            translated_points[j].x = points[j].x;
            translated_points[j].y = points[j].y;
            translated_points[j].z = points[j].z;

            /* Rotate on the x axis */
            points[j].y = cos(player.angle.x) * translated_points[j].y +
                          sin(player.angle.x) * translated_points[j].z;
            
            points[j].z =-sin(player.angle.x) * translated_points[j].y +
                          cos(player.angle.x) * translated_points[j].z;

            translated_points[j].x = points[j].x;
            translated_points[j].y = points[j].y;
            translated_points[j].z = points[j].z;

            /* Rotate on the z axis */
            points[j].y = cos(player.angle.z) * translated_points[j].y +
                          sin(player.angle.z) * translated_points[j].x;

            points[j].x =-sin(player.angle.z) * translated_points[j].y +
                          cos(player.angle.z) * translated_points[j].x;
        }

        /*
        if (points[0].z <= 0 && points[1].z <= 0 && points[2].z <= 0)
            continue;
        */

        /* Check if the points are outside of the bounds of the clipping planes */

        /* Project points onto a 2d plane */
        xcb_point_t triangle_points[3];

        for (j = 0; j < 3; j++) {
            triangle_points[j].x = (screen_size[0]/2) + (display_z/points[j].z)*points[j].x;
            triangle_points[j].y = (screen_size[1]/2) + (display_z/points[j].z)*points[j].y;
        }

        xcb_point_t triangle_line[] = {
            {0,0},{0,0}
        };

        triangle_line[0] = triangle_points[0];
        triangle_line[1] = triangle_points[1];
        triangle_line[1].x -= triangle_line[0].x;
        triangle_line[1].y -= triangle_line[0].y;
        xcb_poly_line(connection, XCB_COORD_MODE_PREVIOUS, window, foreground, 2, triangle_line);
        triangle_line[0] = triangle_points[1];
        triangle_line[1] = triangle_points[2];
        triangle_line[1].x -= triangle_line[0].x;
        triangle_line[1].y -= triangle_line[0].y;
        xcb_poly_line(connection, XCB_COORD_MODE_PREVIOUS, window, foreground, 2, triangle_line);
        triangle_line[0] = triangle_points[2];
        triangle_line[1] = triangle_points[0];
        triangle_line[1].x -= triangle_line[0].x;
        triangle_line[1].y -= triangle_line[0].y;
        xcb_poly_line(connection, XCB_COORD_MODE_PREVIOUS, window, foreground, 2, triangle_line);
    }

    xcb_flush(connection);
}

void
move(Direction dir) {
    /* If the direction is left or right */
    if (dir & DIR_MASK) {
        player.pos.x -= movement_constants[dir] * sin(-player.angle.y + PI/2) * cos(-player.angle.z);
        player.pos.z -= movement_constants[dir] * cos(-player.angle.y + PI/2);
        player.pos.y -= movement_constants[dir] * sin(-player.angle.z);

    /* 
     * The only other type of movement is
     * forward/backward movement
     */
    } else {
        player.pos.x -= movement_constants[dir] * sin(-player.angle.y);
        player.pos.z -= movement_constants[dir] * cos(-player.angle.y) * cos(-player.angle.x);
        player.pos.y -= movement_constants[dir] * sin(-player.angle.x);
    }
}

void
rotate(Direction dir) {
    /* If the direction is left or right */
    player.angle.x += rotation_constants[dir].x;
    player.angle.y += rotation_constants[dir].y;

    if (player.angle.x >= 2*PI)
        player.angle.x -= 2*PI;
    else if (player.angle.x < 0)
        player.angle.x += 2*PI;

    if (player.angle.y >= 2*PI)
        player.angle.y -= 2*PI;
    else if (player.angle.y < 0)
        player.angle.y += 2*PI;
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

    xcb_get_geometry_reply_t *geom;

    xcb_generic_event_t *event;

    int i, type = 1;
    while (!exit) {
        msleep(1000/60);
        //printf("%f %f %f\n", player.pos.x, player.pos.z, player.angle.y*180/PI);

        if ((geom = xcb_get_geometry_reply(connection, 
                        xcb_get_geometry(connection, window), NULL))) {
            screen_size[0] = geom->width;
            screen_size[1] = geom->height;
        }

        draw(connection, window, foreground, screen, type);

        for (i = 0; i < DIR_CNT; i++) {
            if (movement_mask & (1 << i))
                move(i);
            if (rotation_mask & (1 << i))
                rotate(i);
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
                        case 111:
                            rotation_mask |= 0x1 << UP;
                            break;
                        case 113:
                            rotation_mask |= 0x1 << LEFT;
                            break;
                        case 116:
                            rotation_mask |= 0x1 << DOWN;
                            break;
                        case 114:
                            rotation_mask |= 0x1 << RIGHT;
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
                        case 111:
                            rotation_mask &= ~(0x1 << UP);
                            break;
                        case 113:
                            rotation_mask &= ~(0x1 << LEFT);
                            break;
                        case 116:
                            rotation_mask &= ~(0x1 << DOWN);
                            break;
                        case 114:
                            rotation_mask &= ~(0x1 << RIGHT);
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
