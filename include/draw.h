#pragma once
#include <cstddef>
#include <new>
#include <utility>
#include <vector>
#include <string>  
#include "operations.h" // for the shape_t struct

void updateCanvasTexture(const std::vector<Colour>& canvas, int w, int h);

class Frame
{
    public:
        std::vector<shape_t> all_shapes;
        void visualise_canvas(const std::vector<Colour>& canvas, int displayW, int displayH);
};