#include "igMask.h"

igMask::igMask(const std::string& filepath, double background_lenience)
{
  cv::Mat image = cv::imread(filepath, cv::IMREAD_UNCHANGED);

  if (image.empty())
  {
    std::cerr << "Error loading image" << std::endl;
    return;
  }

  cv::resize(image, image, cv::Size(100, 100));

  int64_t pixel_lenience = static_cast<int64_t>(255 * background_lenience);

  for (int64_t i = 0; i < image.rows; ++i)
  {
    cv::Vec3b* ptr = image.ptr<cv::Vec3b>(i);
    for (int64_t j = 0; j < image.cols; ++j)
    {
      uchar blue = ptr[j][0];
      uchar green = ptr[j][1];
      uchar red = ptr[j][2];

      if (blue <= pixel_lenience && green <= pixel_lenience && red <= pixel_lenience)
        m_data.push_back(igVec2(i, j));
    }
  }
}
