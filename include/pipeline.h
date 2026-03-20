#pragma once
<<<<<<< HEAD
#include <cstddef>
#include <new>
#include <utility>
#include <vector>
#include "params.h"


/*
*   Create a black canvas object, this is only run once per execution
*/
std::vector<std::vector<Colour>> createCanvas(int width = IMG_WIDTH, int height = IMG_HEIGHT);
=======
#include <vector>
#include "params.h"
#include "arena.h"
#include "operations.h"


extern std::atomic<int> currentShape;

std::vector<Colour> createCanvas(int width = IMG_WIDTH, int height = IMG_HEIGHT);
>>>>>>> ff2dc14 (Finished Base Model)

/*
* # find_best_shape()
* ## Description
<<<<<<< HEAD
* - Finds the best shape that fits the allocated canvas within a defined number of generations
* ## Inputs
* - `vector<vector<Colour> canvas` -> The current canvas state.
* - `int num_generations` -> The number of generations that is taken to optimise the shape
* - `vector<vector<vector<int>>> all_masks` -> All of the available shape masks, these are defined in shapes/
* ## Outputs
* - `shape_t : best_shape` -> the shape that has performed the best in terms of similarity to the canvas
*/
shape_t find_best_shape(std::vector<std::vector<Colour>> canvas, std::vector<std::vector<std::vector<int>>> all_masks, int num_generations = NUM_GENERATIONS);
=======
* - Finds the best shape to add to the canvas by running multiple generations
* ## Inputs
* - `const vector<vector<Colour>>& canvas` -> The current state of the canvas
* - `const vector<vector<vector<int>>>& all_masks` -> All available shape masks
* - `const vector<vector<Colour>>& target_image` -> The target image to approximate
* - `int num_generations` -> Number of generations to run
* ## Outputs
* - `shape_t` -> The best shape found
*/

std::vector<Colour> resizeCanvas(const std::vector<Colour>& canvas,
    int newW,
    int newH);
>>>>>>> ff2dc14 (Finished Base Model)


/*
* # draw_shapes()
* ## Description
* - Finds the a number `n` best shapes and draws all of those shapes to the canvas
* ## Inputs
<<<<<<< HEAD
* - `vector<vector<Colour>> canvas` -> The current state of the canvas
=======
* - `const vector<vector<Colour>>& canvas` -> The current state of the canvas
* - `const vector<vector<vector<int>>>& all_masks` -> All available shape masks
>>>>>>> ff2dc14 (Finished Base Model)
* - `int num_shapes` -> The number of shapes that will be drawn to the canvas
* ## Outputs
* - `bool` -> Returns True if the canvas has drawn successfully otherwise returns False
*/
<<<<<<< HEAD
bool draw_shapes(std::vector<std::vector<Colour>> canvas, std::vector<std::vector<std::vector<int>>> all_masks, int num_shapes = NUM_SHAPES);

=======
bool draw_shapes(
    const std::vector<Colour>& canvas,
    const std::vector<std::vector<ShapePoint>>& all_masks,
    const std::string& filepath,
    int num_shapes = NUM_SHAPES);
>>>>>>> ff2dc14 (Finished Base Model)
