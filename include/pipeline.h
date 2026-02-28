#pragma once
#include <cstddef>
#include <new>
#include <utility>
#include <vector>
#include <params.h>

/*
*   Create a black canvas object, this is only run once per execution
*/
std::vector<std::vector<Colour>> createCanvas(int width = IMG_WIDTH, int height = IMG_HEIGHT)

/*
* Find the best shape across n Generations
*/
void find_best_shape(int num_generations = NUM_GENERATIONS)


/*
* Draw all the shapes on to the canvas
*/
void draw_shapes(int num_shapes = NUM_SHAPES)

