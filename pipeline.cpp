#include <iostream>
#include <vector>
#include <include/arena.h>
#include <random>

#include "include/params.h"
#include "include/operations.h"
#include "include/generation.h"
#include "include/draw.h"
#include <cstdlib>
using namespace std;

/*
This folder contains general pipeline functinos that may not already exist in other functions, this includes a general function for finding the best shape and drawing it on to the canvas
*/



std::vector<std::vector<Colour>> createCanvas(int width = IMG_WIDTH, int height = IMG_HEIGHT)
{
    Colour black;
    black.red = 0; black.blue = 0; black.green = 0;

    std::vector<std::vector<Colour>> canvas;

    // Start the background at black
    for (int x = 0; x < IMG_WIDTH; x++) 
    {
        std::vector<Colour> row;
        for (int y = 0; y < IMG_HEIGHT; y++)
        {
            row.push_back(black);
        }

        canvas.push_back(row);
    }


    return canvas;
}

shape_t find_best_shape(std::vector<std::vector<Colour>> canvas, std::vector<std::vector<std::vector<int>>> all_masks, int num_generations = NUM_GENERATIONS)
{
    shape_t top_shape;
    for (int i = 0; i < num_generations; i++) {
        top_shape = createGeneration(top_shape, canvas, all_masks);
    }
    return top_shape;
}

bool draw_shapes(vector<vector<Colour>> canvas, std::vector<std::vector<std::vector<int>>> all_masks, int num_shapes = NUM_SHAPES) 
{
    Frame frame;

    vector<vector<Colour>> canvasCopy = canvas;

    for (int shape = 0; shape < num_shapes; shape++)
    {
        shape_t best_shape = find_best_shape(canvasCopy, all_masks);
        project_canvas(canvasCopy, best_shape);

        Frame.all_shapes.push_back(best_shape);
    }

    Frame.visualise_canvas();

    return true; // Returns True if the function runs successfully
}


