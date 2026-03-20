#include "params.h"
#include "operations.h"
#include <vector>
#include <mutex>
#include <atomic>

extern std::vector<Colour> pendingCanvas;
extern std::atomic<bool> canvasDirty;
extern std::mutex canvasMutex;
extern int pendingW, pendingH;

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
void project_canvas(std::vector<Colour> &canvas, const shape_t& shape);

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
    const shape_t& bestShape,
    const std::vector<Colour>& canvas,
    const std::vector<std::vector<ShapePoint>>& all_masks,
    const std::vector<Colour>& target,
    const int generation_number,
    const int desample_rate
);
