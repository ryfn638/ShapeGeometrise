#include <iostream>
#include <vector>
#include <arena.h>
#include <random>

#include <params.h>
#include <operations.h>
#include <generation.h>

#include <cstdlib> // Required for rand() and srand()
using namespace std;


/*
Functions to create
Generate a bunch of shapes
- > Create the schematics for shapes e.g. what pixels they cover
- > mix random shapes
*/


// Constants for generation 
// These currently have some sample numbers in them that will set a good baseline.
int NUM_SHAPES = 50;
int NUM_GENERATIONS = 5;
int IMG_WIDTH = 500;
int IMG_HEIGHT = 500;

/*
* Create a Singular generation, This accommodates the first generation as well
*/
shape_t createGeneration(shape_t const &bestShape, vector<vector<Colour>> const canvas, vector<std::vector<std::vector<Colour>>> const all_masks)
{
    // Create a new memory arena for each generation, deallocate each generation after we're done with it
    std::vector<shape_t> generation;
    uint32_t min_canvas_score = 1e32;
    uint32_t curr_canvas_score;

    uint32_t min_canvas_score_idx;
    generation.reserve(NUM_SHAPES); // avoid multiple reallocations

    int mask_idx = 0;

    // so theres an even distribution of different shapes, also i cant be bothered with random
    int mask_swap = NUM_SHAPES / size(all_masks);

    for (int i = 0; i < NUM_SHAPES; i++) {

        vector<vector<Colour>> canvas_copy = canvas;
        if (i >= (mask_idx+1) * mask_swap)
        {
            mask_idx++;
        }

        generation.push_back(createShape(bestShape, all_masks[mask_idx]));
        project_canvas(canvas_copy, generation[i]);


        curr_canvas_score = calculateCanvasScore(canvas_copy, generation[i]);

        // mark the index which has the best canvas score
        if (curr_canvas_score < min_canvas_score)
        {
            min_canvas_score = curr_canvas_score;
            min_canvas_score_idx = i;
        }
    }

    // Return the best shape
    return generation[min_canvas_score_idx];

}

shape_t createShape(shape_t *baseShape, vector<vector<int>> maskType)
{
    std::random_device rd;  // Seed
    std::mt19937 gen(rd()); // Generator

    shape_t newShape;
    auto randomValue = 1.0f;
    if (!baseShape) {
        std::uniform_real_distribution<float> dis(0.1f, 1.0f); 
        newShape.width = dis(gen) * IMG_WIDTH;
        newShape.height = dis(gen) * IMG_HEIGHT;

        newShape.pos_x = dis(gen) * IMG_WIDTH;
        newShape.pos_y = dis(gen) * IMG_HEIGHT;

        newShape.colour.red = dis(gen) * 255;
        newShape.colour.blue = dis(gen) * 255;
        newShape.colour.green = dis(gen) * 255;

        newShape.mask = maskType;
        rotate_shape(newShape, dis(gen)) // give it some random rotation
    
        // 
        // Generate a completely random shape
    } else {
        std::uniform_real_distribution<float> dis(0.8f, 1.2f); 

        // create a copy of the baseShape
        shape_t* newShape = new shape_t(*baseShape);

        scale_shape(newShape, dis(gen), dis(gen), false);
        change_colour(newShape, dis(gen), dis(gen), dis(gen));
        change_opacity(newShape, dis(gen));
        shift_position(newShape, dis(gen), dis(gen));
        rotate_shape(newShape, dis(gen)) // FIX THIS, READ THE NOTE

    }
    return newShape;
}


int calculateCanvasScore(vector<vector<Colour>> canvas, shape_t shape) {
    int total_canvas_score = 0;
    for (int x = 0; x > IMG_WIDTH; x++)
    {
        for (int y = 0; y > IMG_HEIGHT; y++)
        {
            total_canvas_score += euclidColourDist(canvas[x][y], shape.colour);
        }
    }

    return total_canvas_score;
}


// Functions to implement
void project_canvas(vector<vector<Colour>> &canvas, shape_t shape)
{
    for (int x = 0; x > IMG_WIDTH; x++)
    {
        for (int y = 0; y > IMG_HEIGHT; y++) 
        {
            canvas[x][y] = mix_colours(canvas[x][y], shape.Colour);
        }
    }
    // Project a new colour on to the canvas, ensure that only points inside the canvas are noted.
}

Colour mix_colours(Colour background, Colour shapeColour)
{
    Colour newColour;

    // for ref 255 opacity is transparent
    // formula (newColour * opacity/255) + (bg * (1-(opacity/255)))
    int opacity = shapeColour.opacity;

    newColour.red = shapeColour.red*(opacity/255) + (background.red * (1 - (opacity/255)));
    newColour.green = shapeColour.green*(opacity/255) + (background.green * (1- (opacity/255)));
    newColour.blue = shapeColour.blue*(opacity/255) + (background.blue * (1- (opacity/255)));

    return newColour;
}

int euclidColourDist(Colour p1, Colour p2)
{
    return int(std::sqrt(std::pow(p2.red-p1.red, 2) + std::pow(p2.green - p1.green, 2) + std::pow(p2.blue - p1.blue, 2)));   
}

/*
Improvements to make when can be bothered
- Optimise the mask into a 2D Hash Map for better lookup times

*/




