#pragma once

#include "math.h"

typedef enum {
    DOWN, UP,
    LEFT, RIGHT,
    DIR_CNT
} Direction;

typedef struct {
    Vector3d pos;
    Vector3d angle;
    /* Masks for which movement keys are pressed */
    char move_keys : 4;
    char rot_keys  : 4;
} Player;

void moveCamera(double magnitude);
void rotateCamera(double magnitude);
