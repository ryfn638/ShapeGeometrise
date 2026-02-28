#pragma once
#include <cstddef>
#include <new>
#include <utility>
#include <vector>

struct Colour
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t opacity;
};

struct shape_t
{
    uint16_t width;
    uint16_t height;
    std::vector<std::vector<int>> mask;
    uint32_t pos_x;
    uint32_t pos_y;
    Colour colour;
};


/*
* Rotates an existing shape by an amount clockwise in radians
* TO DO, BECAUSE THIS DOESNT COMPLY WITH THE SHIFT, YOU WILL NEED TO EITHER
* - FIND THE MID POINT THROUGH CALCULATION. THIS OPTION IS MORE COMPUTATIONALLY INTENSIVE
* CONSIDER THE POSITION X AND Y BEFORE ROTATING
*/
void rotate_shape(shape_t &baseShape, float angleScale);

/*
* Scales an existing shape either up or down
* Locked scaling ensures that both shapes scale in proportion to eachother, aka the aspect ratio remains the same
*/
void scale_shape(shape_t &baseShape, float scaleX, float scaleY, bool lockedScaling);

/*
Slightly adjusts the colour of an existing shape
*/
void change_colour(shape_t &baseShape, float scale_R, float scale_G, float scale_B);

/*
* Creates a mask for an image, this is just used to identify which pixels are covered by the shape
*/
void create_mask(std::vector<std::vector<int>> &mask, string filepath);

/*
* Adjusts the opacity of an existing shape
*/
void change_opacity(shape_t &baseShape, float scale);

void shift_position(shape_t &baseShape, float movePercentage_x, float movePercentage_y)