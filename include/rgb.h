#ifndef RGB_F_H
#define RGB_F_H

#include "progcc_includes.h"

typedef enum{
    RGB_GROUP_RS = 0,
    RGB_GROUP_LS = 1,
    RGB_GROUP_DPAD = 2,
    RGB_GROUP_MINUS = 3,
    RGB_GROUP_CAPTURE = 4,
    RGB_GROUP_HOME = 5,
    RGB_GROUP_PLUS = 6,
    RGB_GROUP_Y = 7,
    RGB_GROUP_X = 8,
    RGB_GROUP_A = 9,
    RGB_GROUP_B = 10,
    RGB_GROUP_MAX,
} rgb_group_t;

void rgb_set_group(rgb_group_t group, uint32_t color);

void rgb_set_all(uint32_t color);

void rgb_init();

void rgb_tick(uint32_t timestamp);

#endif
