#include <operations.h>
#include <vector>
#include <cmath>
#include <params.h>

// Image reading utilities
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>

using namespace std;

struct Point {
    int x;
    int y;
};


/*
* Creates the mask for a shape, this is r
*/
void createMask(std::vector<std::vector<int>> &mask, double backgorund_lenience=0.9, string filepath) {
    cv::Mat image = cv::imread(filepath); // Load the image

    if (image.empty()) {
        std::cerr << "Error loading image" << std::endl;
        return -1;
    }

    // This is the value that the algorithm will start to treat something as background, essentially white pixels or pixels
    // that are very close to white are treated as the background
    int pixel_lenience = 255 * backgorund_lenience;

    for (int i = 0; i < image.rows; ++i) {
    // Get pointer to the start of the current row
        cv::Vec3b* ptr = image.ptr<cv::Vec3b>(i);
        for (int j = 0; j < image.cols; ++j) {
            // Access the pixel data
            uchar blue = ptr[j][0];
            uchar green = ptr[j][1];
            uchar red = ptr[j][2];
            
            if (blue <= pixel_lenience && green <= pixel_lenience && red <= pixel_lenience)
            {
                // If all pixels are under the pixel lenience then the shape is an acceptable form of whtie
                mask.push_back({i, j});
            }
        }
    }


    // For creating masks, pixels that are white will be discredited as background parts.
    // do this last


}

void rotate_shape(shape_t &baseShape, float angle)
{
    Point rotAxis;
    rotAxis.x = baseShape.pos_x + (baseShape.width / 2);
    rotAxis.y = baseShape.pos_y + (baseShape.height / 2);

    for (int i = 0; i<size(baseShape.mask); i++)
    {
        Point curr_point;

        curr_point.x = baseShape.mask[i][0];
        curr_point.y = baseShape.mask[i][1];
        float dist = euclidDist(rotAxis, curr_point);
        float curr_angle = std::atan(curr_point.y - rotAxis.y / curr_point.x - rotAxis.x);
        baseShape.mask[i][0] = dist * std::tan(curr_angle + angle);
        
    }

    return baseShape;
}
void scale_shape(shape_t baseShape, float scaleX, float scaleY, bool lockedScaling = false)
{
    // Define the new bounding box
    int newWidth = baseShape.width * scaleX;
    int newHeight = baseShape.height * scaleY;
    std::vector<std::vector<int>> newMask;
    if (lockedScaling) {
        newHeight = baseShape.height * scaleX;
    }


    if (scaleX >= 1.0) {
        newMask = upsample_2d(baseShape.mask, scaleX, scaleY);
    } else {
        newMask = downsample_2d(baseShape.mask, scaleX, scaleY);
    }
    // Bicubic interpolation if scaling upwards
    // Area interpolation if scaling downwards

    baseShape.mask = newMask;

    return baseShape;
}

std::vector<std::vector<int>> upsample_2d(std::vector<std::vector<int>> mask, float scaleX, float scaleY)
{
    /*
        Could do make a 2D Hash in here temporarily and deallocate it when we dont need it
        This is just so when I check if a current shape already exists then I dont have cancer lookup times
    */


    for (int i = 0; i<size(mask); i++)
    {
        Point newPoint;
        newPoint.x = mask[i][0] * scaleX;
        newPoint.y = mask[i][1] * scaleY;
    
        // if not fonud in the list append it on, 
        if (search_vec(newPoint, mask) >= 0)
        {
            mask.push_back(std::vector<int> {newPoint.x, newPoint.y});
        }

    }

    return mask;
}

std::vector<std::vector<int>> downsample_2d(std::vector<std::vector<int>> mask, float scaleX, float scaleY)
{
    // Dont know how downsampling works but im assuming i just scale down the coordinate and check if it is in my mask.
    // if it is in my mask I just remove the value, if it isnt then 


    for (int i = 0; i<size(mask); i++)
    {
        Point newPoint;
        newPoint.x = mask[i][0] * scaleX;
        newPoint.y = mask[i][1] * scaleY;
    
        // if not fonud in the list append it on, 
        int foundidx = search_vec(newPoint, mask);

        if (foundidx >= 0)
        {
            mask.erase(mask.begin() + foundidx);
        } else {
            mask.push_back(std::vector<int> {newPoint.x, newPoint.y});
        }
    }



    return mask;
    
}

int search_vec(Point subject, std::vector<std::vector<int>> mask)
{
    for (int i = 0; i < size(mask); i++)
    {
        if (subject.x == mask[i][0] && subject.y == mask[i][1]) {
            return i;
        }
    }

    return -1;
}

void change_colour(shape_t baseShape, float scale_R, float scale_G, float scale_B)
{
    Colour newColour;
    newColour.red = std::min(255, baseShape.colour.red * scale_R);
    newColour.green = std::min(255, baseShape.colour.green * scale_G);
    newColour.blue = std::min(255, baseShape.colour.blue * scale_B);
    
    baseShape.colour = newColour;

    return baseShape;
}

void change_opacity(shape_t baseShape, float scale)
{
    Colour newColour = baseShape.colour;
    newColour.opacity *= scale;
    newColour.opacity = std::min(255, newColour.opacity)


    baseShape.colour = newColour;
    return baseShape;
}

void shift_position(shape_t baseShape, float movePercentage_x, float movePercentage_y)
{
    int pixels_moved_x = movePercentage_x * IMG_WIDTH;
    int pixels_moved_y = movePercentage_y * IMG_HEIGHT;

    for (int i = 0; i < size(baseShape.mask); i++)
    {
        baseShape.mask[i][0] += pixels_moved_x;
        baseShape.mask[i][1] += pixels_moved_y;
    }

    return baseShape;

}
float euclidDist(Point p1, Point p2)
{
    return std::sqrt(std::pow(p2.x-p1.x, 2) + std::pow(p2.y - p1.y, 2));   
}


