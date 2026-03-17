#include <iostream>
#include <vector>
#include <include/arena.h>
#include <random>

#include "include/params.h"
#include "include/operations.h"
#include "include/generation.h"
#include "include/draw.h"
using namespace std;


/*
Dump all the OpenGL code here when I can be bothered to learn it
*/

class Frame
{
    vector<shape_t> all_shapes;
    /*
    * Draw each shape on to an empty black canvas. This is visaulised through OpenGL.
    * hey maybe could do this in 3 dimensions perhaps???
    */
    void visualise_canvas();
}


