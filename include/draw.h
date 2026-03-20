#pragma once
#include <cstddef>
#include <new>
#include <utility>
#include <vector>
#include <string>  
#include "operations.h" // for the shape_t struct

<<<<<<< HEAD
class Frame
{
    std::vector<shape_t> all_shapes;


    /*
    * Draw each shape on to an empty black canvas. This is visaulised through OpenGL.
    * hey maybe could do this in 3 dimensions perhaps???
    */
    void visualise_canvas();
=======
void updateCanvasTexture(const std::vector<Colour>& canvas, int w, int h);

class Frame
{
    public:
        std::vector<shape_t> all_shapes;
        void visualise_canvas(const std::vector<Colour>& canvas, int displayW, int displayH);
>>>>>>> ff2dc14 (Finished Base Model)
};