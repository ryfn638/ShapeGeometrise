#include "params.h"
#include "operations.h"
#include <vector>


/*
# project_canvas
* ### Description:
* Projects a specified shape onto the canvas and modifies the inputted canvas

* ### Inputs:
* - vector<vector<Colour> : &canvas -> The current canvas that the user wants to modify
* - shape_t: shape -> The shape that the user wants to project on to the canvas
*
* ### Outputs:
* - None -> The canvas is modified through the & key
*/
<<<<<<< HEAD
void project_canvas(std::vector<std::vector<Colour>> &canvas, shape_t shape);
=======
void project_canvas(std::vector<Colour> &canvas, const shape_t& shape);
>>>>>>> ff2dc14 (Finished Base Model)

/*
# createGeneration()
* ### Description:
* Creates a generation of size NUM_SHAPES in params.h and outputs the best performing shape in terms of canvas score in that generation

* ### Inputs:
* - shape_t: bestShape -> The shape which performed the best in the previous generation
* - vect<vector<Colour>>: canvas -> The canvas at the time of the generation
* - vector<vector<vector<Colour>>> collection of all the different shape masks that are being used.

* ### Outputs:
* - shape_t: bestShape -> The shape that has performed the best in the generation in terms of canvas score
*/
shape_t createGeneration(
<<<<<<< HEAD
    shape_t const &bestShape,
    std::vector<std::vector<Colour>> const canvas,
    std::vector<std::vector<std::vector<int>>> const all_masks);
=======
    const shape_t& bestShape,
    const std::vector<Colour>& canvas,
    const std::vector<std::vector<ShapePoint>>& all_masks,
    const std::vector<Colour>& target,
    const int generation_number,
    const int desample_rate
);
>>>>>>> ff2dc14 (Finished Base Model)
