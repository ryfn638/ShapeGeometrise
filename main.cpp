#include <iostream>
#include <vector>
#include "include/arena.h"
#include <random>

#include "include/params.h"
#include "include/operations.h"
#include "include/generation.h"
#include "include/draw.h"
#include "include/pipeline.h"

#include <cstdlib>


#include <filesystem>
#include <string>

using namespace std;

namespace fs = std::filesystem;


/*
Code written under MIT License by Ryan Fraser
This is my first usage of c++ for this library

Any and all feedback is appreciated
*/

int main()
{

    vector<vector<vector<int>>> all_masks;
    try {
        // Iterate over all entries in the directory
        for (const auto& entry : fs::directory_iterator("shapes")) {
            std::vector<std::vector<int>> mask;
            create_mask(mask, entry.path().string());
            all_masks.push_back(mask);

        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    }

    // Create the canvas
    vector<vector<Colour>> canvas = createCanvas();

    // Find the best shapes and draw them
    find_best_shape();

}