//// ui.cpp
//#include "include/ui.h"
//#include "include/params.h"
//#include <windows.h>
//#include <commdlg.h>
//#include <opencv2/highgui.hpp>
//#include <opencv2/imgproc.hpp>
//#include <iostream>
//#include <string>
//
//static std::string selectedFilepath = "";
//static bool shouldGenerate = false;
//
//std::string openFileDialog()
//{
//    char filename[MAX_PATH] = "";
//    OPENFILENAMEA ofn = {};
//    ofn.lStructSize = sizeof(ofn);
//    ofn.hwndOwner = NULL;
//    ofn.lpstrFilter = "Image Files\0*.jpg;*.jpeg;*.png;*.bmp\0All Files\0*.*\0";
//    ofn.lpstrFile = filename;
//    ofn.nMaxFile = MAX_PATH;
//    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
//
//    if (GetOpenFileNameA(&ofn))
//        return std::string(filename);
//    return "";
//}
//
//// button click callback
//static void onMouse(int event, int x, int y, int, void* userdata)
//{
//    if (event != cv::EVENT_LBUTTONDOWN) return;
//
//    cv::Mat* ui = (cv::Mat*)userdata;
//
//    // select image button: 20,20 -> 180,60
//    if (x >= 20 && x <= 180 && y >= 20 && y <= 60)
//    {
//        selectedFilepath = openFileDialog();
//        if (!selectedFilepath.empty())
//        {
//            // show filename on UI
//            cv::rectangle(*ui, { 20, 80 }, { 380, 110 }, { 40, 40, 40 }, -1);  // clear old text
//            std::string fname = selectedFilepath.substr(selectedFilepath.find_last_of("/\\") + 1);
//            cv::putText(*ui, fname, { 20, 105 }, cv::FONT_HERSHEY_SIMPLEX, 0.5, { 200, 200, 200 }, 1);
//            cv::imshow("Geometrise Setup", *ui);
//        }
//    }
//
//    // generate button: 20,260 -> 180,300  -- only active if file selected
//    if (x >= 20 && x <= 180 && y >= 260 && y <= 300 && !selectedFilepath.empty())
//    {
//        shouldGenerate = true;
//    }
//}
//
//std::string showParamsUI()
//{
//    cv::Mat ui(320, 400, CV_8UC3, cv::Scalar(40, 40, 40));
//    cv::namedWindow("Geometrise Setup", cv::WINDOW_AUTOSIZE);
//    cv::setMouseCallback("Geometrise Setup", onMouse, &ui);
//
//    // select image button
//    cv::rectangle(ui, { 20, 20 }, { 180, 60 }, { 80, 80, 80 }, -1);
//    cv::putText(ui, "Select Image", { 30, 48 }, cv::FONT_HERSHEY_SIMPLEX, 0.6, { 220, 220, 220 }, 1);
//
//    // filepath label placeholder
//    cv::putText(ui, "No image selected", { 20, 105 }, cv::FONT_HERSHEY_SIMPLEX, 0.5, { 160, 160, 160 }, 1);
//
//    // trackbars
//    cv::createTrackbar("Shapes", "Geometrise Setup", &NUM_SHAPES, 500);
//    cv::createTrackbar("Gen Shapes", "Geometrise Setup", &GENERATION_SHAPES, 300);
//    cv::createTrackbar("Generations", "Geometrise Setup", &NUM_GENERATIONS, 10);
//
//    // generate button
//    cv::rectangle(ui, { 20, 260 }, { 180, 300 }, { 40, 120, 40 }, -1);
//    cv::putText(ui, "Generate", { 40, 288 }, cv::FONT_HERSHEY_SIMPLEX, 0.7, { 220, 220, 220 }, 1);
//
//    cv::imshow("Geometrise Setup", ui);
//
//    // wait until generate is clicked
//    while (!shouldGenerate)
//    {
//        int key = cv::waitKey(50);
//        if (key == 27) return "";  // ESC to cancel
//    }
//
//    cv::destroyWindow("Geometrise Setup");
//    return selectedFilepath;
//}