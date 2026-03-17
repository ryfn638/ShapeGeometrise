#pragma once
#include <cstddef>
#include <new>
#include <utility>
#include <vector>
#include <string>  
#include "operations.h" // for the shape_t struct

class Frame
{
    std::vector<shape_t> all_shapes;


    /*
    * Draw each shape on to an empty black canvas. This is visaulised through OpenGL.
    * hey maybe could do this in 3 dimensions perhaps???
    */
    void visualise_canvas();
};