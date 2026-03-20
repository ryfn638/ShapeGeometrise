#include <iostream>
#include <vector>
#include "include/arena.h"
#include <random>

#include "include/params.h"
#include "include/operations.h"
#include "include/generation.h"
#include "include/draw.h"

#include <cstdlib>
#include "include/pipeline.h"
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
using namespace std;

/*
This folder contains general pipeline functinos that may not already exist in other functions, this includes a general function for finding the best shape and drawing it on to the canvas
*/

// helper to access 1D canvas
inline Colour& getPixel(std::vector<Colour>& canvas, int x, int y) {
    return canvas[y * IMG_WIDTH + x];
}
inline const Colour& getPixel(const std::vector<Colour>& canvas, int x, int y) {
    return canvas[y * IMG_WIDTH + x];
}


std::vector<Colour> createCanvas(int width, int height)
{
    return std::vector<Colour>(width * height, { 0, 0, 0, 255 });
}

shape_t find_best_shape(
    const std::vector<Colour>& canvas,
    const std::vector<std::vector<ShapePoint>>& all_masks,
    const std::vector<Colour>& target_image,
    int num_generations,
    int desample_rate = 2)
{
    shape_t top_shape;
    for (int i = 0; i < num_generations; i++) {
        top_shape = createGeneration(top_shape, canvas, all_masks, target_image, i, desample_rate);
    }
    return top_shape;
}

std::vector<Colour> resizeCanvas(const std::vector<Colour>& canvas, int newW, int newH)
{
    // convert to cv::Mat, resize, convert back
    cv::Mat img(IMG_HEIGHT, IMG_WIDTH, CV_8UC3);
    for (int x = 0; x < IMG_WIDTH; x++)
        for (int y = 0; y < IMG_HEIGHT; y++) {
            const Colour& c = getPixel(canvas, x, y);
            img.at<cv::Vec3b>(y, x) = { c.blue, c.green, c.red };
        }

    cv::Mat resized;
    cv::resize(img, resized, cv::Size(newW, newH));

    std::vector<Colour> newCanvas(newW * newH);
    for (int x = 0; x < newW; x++)
        for (int y = 0; y < newH; y++) {
            cv::Vec3b p = resized.at<cv::Vec3b>(y, x);
            newCanvas[y * newW + x] = { p[2], p[1], p[0], 255 };
        }
    return newCanvas;
}

// in draw_shapes, scale target and canvas based on progress
bool draw_shapes(
    const std::vector<Colour>& canvas,
    const std::vector<std::vector<ShapePoint>>& all_masks,
    const std::string& filepath,
    int num_shapes)
{
    Frame frame;
    Frame viewFrame;
    std::vector<Colour> canvasCopy = canvas;
    std::vector<Colour> displayCanvas = canvas;
    std::vector<Colour> target_image;

    const int ORIG_WIDTH = IMG_WIDTH;
    const int ORIG_HEIGHT = IMG_HEIGHT;

    // load target once at full res
    cv::Mat targetFull = cv::imread(filepath);
    cv::resize(targetFull, targetFull, cv::Size(ORIG_WIDTH, ORIG_HEIGHT));

    float currentScale = -1.0f;

    struct Stage { float progress, scale; int subsample; };
    std::vector<Stage> stages = {
        { 0.0f, 0.25f, 8 },
        { 0.3f, 0.5f,  4 },
        { 0.6f, 0.75f, 2 },
        { 0.8f, 1.0f,  1 },
    };

    for (int s = 0; s < num_shapes; s++)
    {
        float progress = (float)s / num_shapes;

        Stage currentStage = stages[0];
        for (const auto& stage : stages)
            if (progress >= stage.progress)
                currentStage = stage;

        if (currentStage.scale != currentScale)
        {
            currentScale = currentStage.scale;
            int scaledW = std::max(1, int(ORIG_WIDTH * currentScale));
            int scaledH = std::max(1, int(ORIG_HEIGHT * currentScale));

            canvasCopy = resizeCanvas(canvasCopy, scaledW, scaledH);
            IMG_WIDTH = scaledW;
            IMG_HEIGHT = scaledH;

            // resize cached full res target instead of reloading from disk
            cv::Mat targetScaled;
            cv::resize(targetFull, targetScaled, cv::Size(scaledW, scaledH));

            target_image.resize(scaledW * scaledH);
            for (int x = 0; x < scaledW; x++)
                for (int y = 0; y < scaledH; y++) {
                    cv::Vec3b p = targetScaled.at<cv::Vec3b>(y, x);
                    target_image[y * scaledW + x] = { p[2], p[1], p[0], 255 };
                }

            currentShape = s;
        }

        shape_t best_shape = find_best_shape(canvasCopy, all_masks, target_image, NUM_GENERATIONS);
        project_canvas(canvasCopy, best_shape);
        frame.all_shapes.push_back(best_shape);

        shape_t displayShape = best_shape;
        float upscale = 1.0f / currentScale;
        displayShape.pos_x = int(best_shape.pos_x * upscale);
        displayShape.pos_y = int(best_shape.pos_y * upscale);
        scale_shape(displayShape, upscale, upscale, false);
        rotate_shape(displayShape, best_shape.angle);


        int savedW = IMG_WIDTH;
        int savedH = IMG_HEIGHT;
        IMG_WIDTH = ORIG_WIDTH;
        IMG_HEIGHT = ORIG_HEIGHT;

        project_canvas(displayCanvas, displayShape);
        viewFrame.visualise_canvas(displayCanvas, ORIG_WIDTH, ORIG_HEIGHT);

        IMG_WIDTH = savedW;
        IMG_HEIGHT = savedH;

        std::cout << s << std::endl;
    }

    cv::waitKey(0);
    return true;
}

