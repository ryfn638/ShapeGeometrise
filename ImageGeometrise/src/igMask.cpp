#include "igMask.h"
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

igMask::igMask(int64_t width, int64_t height)
  : m_width(width),
    m_height(height),
    m_data(width * height, 1)
{
}

igMask::igMask(const std::string& filepath, double background_lenience)
{
  // IMREAD_COLOR so it's always 3 channels; a PNG with alpha would be 4 and break the Vec3b reads
  cv::Mat image = cv::imread(filepath, cv::IMREAD_COLOR);

  if (image.empty())
  {
    std::cerr << "Error loading image " << filepath << std::endl;
    return;
  }

  m_width = image.cols;
  m_height = image.rows;
  m_data.assign(m_width * m_height, 0);

  int64_t pixel_lenience = static_cast<int64_t>(255 * background_lenience);

  for (int64_t y = 0; y < m_height; ++y)
  {
    const cv::Vec3b* ptr = image.ptr<cv::Vec3b>(int(y));
    for (int64_t x = 0; x < m_width; ++x)
    {
      uchar blue = ptr[x][0];
      uchar green = ptr[x][1];
      uchar red = ptr[x][2];

      if (blue <= pixel_lenience && green <= pixel_lenience && red <= pixel_lenience)
      {
        m_data[y * m_width + x] = 1;
      }
    }
  }
}
