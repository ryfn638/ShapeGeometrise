#include "include/operations.h"
#include <vector>
#include <cmath>
#include "include/params.h"
#include <string>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <opencv2/imgproc.hpp>
#include <unordered_set>
using namespace std;
using namespace cv;  // added cv namespace

using namespace std;



/* ----------------------------
*
*   GENERAL TOOLS
*
* ----------------------------- */


/*
Searches a mask 2D vector to see if a ShapePoint is contained, if not then returns -1
else returns the index.
*/
int search_vec(ShapePoint subject, std::vector<std::vector<int>> mask)
{
    for (int i = 0; i < size(mask); i++)
    {
        if (subject.x == mask[i][0] && subject.y == mask[i][1]) {
            return i;
        }
    }

    return -1;
}

/*
* Returns the Euclidean Distance between two ShapePoint structs
*/
float euclidDist(ShapePoint p1, ShapePoint p2)
{
    int dx = p2.x - p1.x;
    int dy = p2.y - p1.y;
    return dy * dy + dx * dx;
}




/* --------------------------
*
*   MASK CREATION
* 
* ---------------------------*/
void change_opacity(shape_t& baseShape, float scale)
{
    baseShape.colour.opacity = uint8_t(std::min(255, int(baseShape.colour.opacity * scale)));
}

void create_mask(std::vector<ShapePoint>& mask, string filepath, double background_lenience) {
    cv::Mat image = cv::imread(filepath, cv::IMREAD_UNCHANGED);

    if (image.empty()) {
        std::cerr << "Error loading image" << std::endl;
        return;
    }

    // normalise to unit size so scale_shape works correctly
    cv::resize(image, image, cv::Size(100, 100));

    int pixel_lenience = 255 * background_lenience;

    for (int i = 0; i < image.rows; ++i) {
        Vec3b* ptr = image.ptr<Vec3b>(i);
        for (int j = 0; j < image.cols; ++j) {
            uchar blue = ptr[j][0];
            uchar green = ptr[j][1];
            uchar red = ptr[j][2];

            if (blue <= pixel_lenience && green <= pixel_lenience && red <= pixel_lenience)
                mask.push_back({ (int16_t)i, (int16_t)j });
        }
    }
}

std::vector<Colour> loadTarget(const std::string& path) {
    cv::Mat img = cv::imread(path);
    cv::resize(img, img, cv::Size(IMG_WIDTH, IMG_HEIGHT));

    std::vector<Colour> target(IMG_WIDTH * IMG_HEIGHT);

    for (int x = 0; x < IMG_WIDTH; x++) {
        for (int y = 0; y < IMG_HEIGHT; y++) {
            cv::Vec3b pixel = img.at<cv::Vec3b>(y, x);
            target[y * IMG_WIDTH + x] = { pixel[2], pixel[1], pixel[0] };
        }
    }
    return target;
}

/* ----------------------------
*
*   SHAPE OPERATIONS
*   
* ---------------------------- */
void rotate_shape(shape_t& baseShape, float angle)
{
    float centerX = baseShape.width / 2.0f;
    float centerY = baseShape.height / 2.0f;
    float cosA = std::cos(angle);
    float sinA = std::sin(angle);

    baseShape.angle = angle;
    for (auto& coord : baseShape.mask)
    {
        float dy = coord.y - centerY;
        float dx = coord.x - centerX;
        coord.y = int16_t(centerY + dy * cosA - dx * sinA);
        coord.x = int16_t(centerX + dy * sinA + dx * cosA);
    }
}

void scale_shape(shape_t& baseShape, float scaleX, float scaleY, bool lockedScaling)
{
    if (baseShape.width == 0 || baseShape.height == 0) return;

    float newWidth = std::max(1.0f, baseShape.width * scaleX);
    float newHeight = std::max(1.0f, baseShape.height * (lockedScaling ? scaleX : scaleY));

    std::vector<ShapePoint> newMask;
    newMask.reserve(int(newWidth) * int(newHeight));

    for (int x = 0; x < int(newWidth); x++)
        for (int y = 0; y < int(newHeight); y++)
            newMask.push_back({ int16_t(y), int16_t(x) });

    baseShape.mask = std::move(newMask);
    baseShape.width = newWidth;
    baseShape.height = newHeight;
}

void shift_position(shape_t& baseShape, float movePercentage_x, float movePercentage_y)
{
    int pixels_moved_x = movePercentage_x * IMG_WIDTH;
    int pixels_moved_y = movePercentage_y * IMG_HEIGHT;
    baseShape.pos_x += pixels_moved_x;
    baseShape.pos_y += pixels_moved_y;

   
}




