#pragma once
#include <cstddef>
#include <new>
#include <utility>
#include <vector>
#include <string>  

class Frame
{
    vector<shape_t> all_shapes;


    /*
    * Draw each shape on to an empty black canvas. This is visaulised through OpenGL.
    * hey maybe could do this in 3 dimensions perhaps???
    */
    void visualise_canvas();
}