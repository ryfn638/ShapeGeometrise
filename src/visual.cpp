#include <iostream>
#include <vector>
#include "include/arena.h"
#include <random>

#include "include/params.h"
#include "include/operations.h"
#include "include/generation.h"
#include "include/draw.h"
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
using namespace std;


/*
Dump all the OpenGL code here when I can be bothered to learn it
*/

void updateCanvasTexture(const std::vector<Colour>& canvas, int w, int h);  // defined in main.cpp

void Frame::visualise_canvas(const std::vector<Colour>& canvas, int displayW, int displayH)
{
    updateCanvasTexture(canvas, displayW, displayH);
}
