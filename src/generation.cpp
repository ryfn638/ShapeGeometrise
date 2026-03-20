#include <iostream>
#include <vector>
#include <random>

#include "include/arena.h"
#include "include/params.h"
#include "include/operations.h"
#include "include/generation.h"

#include <cstdlib> // Required for rand() and srand()
using namespace std;



static std::mt19937 gen(std::random_device{}());

/*
Functions to create
Generate a bunch of shapes
- > Create the schematics for shapes e.g. what pixels they cover
- > mix random shapes
*/


// Constants for generation 
// These currently have some sample numbers in them that will set a good baseline.
int NUM_SHAPES = 300;
int GENERATION_SHAPES = 100;
int NUM_GENERATIONS = 2;
int IMG_WIDTH = 274;
int IMG_HEIGHT = 184;


// helper to access 1D canvas
inline Colour& getPixel(std::vector<Colour>& canvas, int x, int y) {
    return canvas[y * IMG_WIDTH + x];
}
inline const Colour& getPixel(const std::vector<Colour>& canvas, int x, int y) {
    return canvas[y * IMG_WIDTH + x];
}

/*
* Create a Singular generation, This accommodates the first generation as well
*/

Colour getAverageColour(const std::vector<Colour>& target, const shape_t& shape)
{
    uint64_t totalR = 0, totalG = 0, totalB = 0;
    int count = 0;
    for (const auto& coord : shape.mask)
    {
        int x = coord.x + shape.pos_x;
        int y = coord.y + shape.pos_y;
        if (x < 0 || x >= IMG_WIDTH || y < 0 || y >= IMG_HEIGHT) continue;
        totalR += getPixel(target, x, y).red;
        totalG += getPixel(target, x, y).green;
        totalB += getPixel(target, x, y).blue;
        count++;
    }
    if (count == 0) return { 0, 0, 0, 255 };
    return { uint8_t(totalR / count), uint8_t(totalG / count), uint8_t(totalB / count), 255 };
}

struct LAB { float L, a, b; };

LAB RGBtoLAB(const Colour& c)
{
    // RGB to linear
    float r = c.red / 255.0f;
    float g = c.green / 255.0f;
    float b = c.blue / 255.0f;

    // gamma correction
    r = r > 0.04045f ? std::pow((r + 0.055f) / 1.055f, 2.4f) : r / 12.92f;
    g = g > 0.04045f ? std::pow((g + 0.055f) / 1.055f, 2.4f) : g / 12.92f;
    b = b > 0.04045f ? std::pow((b + 0.055f) / 1.055f, 2.4f) : b / 12.92f;

    // RGB to XYZ (D65 illuminant)
    float x = r * 0.4124564f + g * 0.3575761f + b * 0.1804375f;
    float y = r * 0.2126729f + g * 0.7151522f + b * 0.0721750f;
    float z = r * 0.0193339f + g * 0.1191920f + b * 0.9503041f;

    // normalise by D65 white point
    x /= 0.95047f;
    y /= 1.00000f;
    z /= 1.08883f;

    // XYZ to LAB
    auto f = [](float t) {
        return t > 0.008856f
            ? std::pow(t, 1.0f / 3.0f)
            : (7.787f * t + 16.0f / 116.0f);
        };

    LAB lab;
    lab.L = 116.0f * f(y) - 16.0f;
    lab.a = 500.0f * (f(x) - f(y));
    lab.b = 200.0f * (f(y) - f(z));
    return lab;
}

int euclidColourDist(Colour p1, Colour p2)
{
    LAB lab1 = RGBtoLAB(p1);
    LAB lab2 = RGBtoLAB(p2);

    float dL = lab2.L - lab1.L;
    float da = lab2.a - lab1.a;
    float db = lab2.b - lab1.b;

    return int(dL * dL + da * da + db * db);
}

Colour mix_colours(Colour background, Colour shapeColour)
{
    float a = shapeColour.opacity / 255.0f;
    Colour newColour;
    newColour.red = int(shapeColour.red * a + background.red * (1.0f - a));
    newColour.green = int(shapeColour.green * a + background.green * (1.0f - a));
    newColour.blue = int(shapeColour.blue * a + background.blue * (1.0f - a));
    newColour.opacity = background.opacity;
    return newColour;
}

int calculateImprovement(
    const std::vector<Colour>& canvas,
    const std::vector<Colour>& target,
    const shape_t& shape,
    int subsample = 1)
{
    int improvement = 0;
    for (int i = 0; i < (int)shape.mask.size(); i += subsample)
    {
        int x = shape.mask[i].x + shape.pos_x;
        int y = shape.mask[i].y + shape.pos_y;
        if (x < 0 || x >= IMG_WIDTH || y < 0 || y >= IMG_HEIGHT) continue;
        int before = euclidColourDist(getPixel(canvas, x, y), getPixel(target, x, y));
        Colour blended = mix_colours(getPixel(canvas, x, y), shape.colour);
        int after = euclidColourDist(blended, getPixel(target, x, y));
        improvement += before - after;
    }
    return improvement * subsample;
}

void project_canvas(std::vector<Colour>& canvas, const shape_t& shape)
{
    for (const auto& coord : shape.mask)
    {
        int x = coord.x + shape.pos_x;
        int y = coord.y + shape.pos_y;
        if (x < 0 || x >= IMG_WIDTH || y < 0 || y >= IMG_HEIGHT) continue;
        getPixel(canvas, x, y) = mix_colours(getPixel(canvas, x, y), shape.colour);
    }
}

shape_t createShape(shape_t* baseShape, 
    const std::vector<ShapePoint>& maskType,
    const std::vector<Colour>& target)
{
    shape_t newShape;
    if (!baseShape) {
        std::uniform_real_distribution<float> dis(0.1f, 0.9f);
        newShape.width = dis(gen) * IMG_WIDTH;
        newShape.height = dis(gen) * IMG_HEIGHT;

        // 0.8 * IMG_WIDTH * 0.05 = always positive drift

        newShape.pos_x = dis(gen) * (IMG_WIDTH - newShape.width);
        newShape.pos_y = dis(gen) * (IMG_HEIGHT - newShape.height);
        newShape.mask = maskType;

        float scaleX = (float)newShape.width / 100.0f;
        float scaleY = (float)newShape.height / 100.0f;
        scale_shape(newShape, scaleX, scaleY, false);
        newShape.colour = getAverageColour(target, newShape);
    }
    else {
        std::uniform_real_distribution<float> dis(0.8f, 1.2f);
        std::uniform_real_distribution<float> disAngle(0.0f, 2.);
        std::uniform_real_distribution<float> disShift(-0.1f, 0.1f);
        newShape = *baseShape;
        scale_shape(newShape, dis(gen), dis(gen), false);
        change_opacity(newShape, dis(gen));
        shift_position(newShape, disShift(gen), disShift(gen));

        rotate_shape(newShape, disAngle(gen));

        newShape.colour = getAverageColour(target, newShape);
    }
    return newShape;
}


/*
Improvements to make when can be bothered
- Optimise the mask into a 2D Hash Map for better lookup times

*/

shape_t createGeneration(
    const shape_t& bestShape,
    const std::vector<Colour>& canvas,
    const std::vector<std::vector<ShapePoint>>& all_masks,
    const std::vector<Colour>& target,
    const int generation_number,
    const int desample_rate = 1)
{
    int max_improvement = INT_MIN;
    int max_improvement_idx = 0;

    std::vector<shape_t> generation(GENERATION_SHAPES);

    #pragma omp parallel for
    for (int i = 0; i < GENERATION_SHAPES; i++) {
        int local_mask_idx = (i * (int)all_masks.size()) / GENERATION_SHAPES;

        shape_t currBestShape = bestShape;
        generation[i] = (generation_number == 0)
            ? createShape(nullptr, all_masks[local_mask_idx], target)
            : createShape(&currBestShape, all_masks[local_mask_idx], target);

        int improvement = calculateImprovement(canvas, target, generation[i], desample_rate);

        //std::cout << improvement << std::endl;
           #pragma omp critical
        {
            if (improvement > max_improvement) {
                max_improvement = improvement;
                max_improvement_idx = i;
            }
        }
    }

    return generation[max_improvement_idx];
}