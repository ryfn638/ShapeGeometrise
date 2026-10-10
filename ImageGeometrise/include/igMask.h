#ifndef igMask_h
#define igMask_h

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <opencv2/imgproc.hpp>
#include "igCommon.h"

// Small wrapper for what is essentially a bitmap
class igMask
{
  igMask(const std::string &filePath, double backgroundLenience);

private:
  std::vector<igVec2> m_data;
};

#endif
