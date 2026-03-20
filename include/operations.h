#pragma once
#include <cstddef>
#include <new>
#include <utility>
#include <vector>
#include <cstdint>
#include <string>


struct ShapePoint {
    int16_t x;
    int16_t y;
};

struct Colour
{
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
    uint8_t opacity = 255;  // default fully opaque
};

struct shape_t
{
    int32_t width;
    int32_t height;
    std::vector<ShapePoint> mask;
    int32_t pos_x;
    int32_t pos_y;
    Colour colour;
    float angle = 0.0f;
    float scale = 1.0f;
};


/*
<<<<<<< HEAD
=======
* # loadTarget()
* ## Descriptions
*   - Loads a specified target image at a given path
* ## Inputs
*   - `const std::string& path` -> the filepath to the image
* ## Outputs
*   - `vector<vector<Colour>> out` -> target image that is formatted the same as canvas
*/
std::vector<Colour> loadTarget(const std::string& path);
/*
>>>>>>> ff2dc14 (Finished Base Model)
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

void create_mask(std::vector<ShapePoint> &mask, std::string filepath, double backgorund_lenience=0.9);

/*
* Adjusts the opacity of an existing shape
*/
void change_opacity(shape_t &baseShape, float scale);

void shift_position(shape_t &baseShape, float movePercentage_x, float movePercentage_y);