#pragma once
#include <cstddef>
#include <new>
#include <utility>
#include <vector>
#include "params.h"
#include "arena.h"
#include "operations.h"


extern std::atomic<int> currentShape;

std::vector<Colour> createCanvas(int width = IMG_WIDTH, int height = IMG_HEIGHT);

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


std::vector<Colour> resizeCanvas(const std::vector<Colour>& canvas,
    int newW,
    int newH);



/*
* # draw_shapes()
* ## Description
* - Finds the a number `n` best shapes and draws all of those shapes to the canvas
* ## Inputs
* - `int num_shapes` -> The number of shapes that will be drawn to the canvas
* ## Outputs
* - `bool` -> Returns True if the canvas has drawn successfully otherwise returns False
*/

bool draw_shapes(
    const std::vector<Colour>& canvas,
    const std::vector<std::vector<ShapePoint>>& all_masks,
    const std::string& filepath,
    int num_shapes = NUM_SHAPES);

