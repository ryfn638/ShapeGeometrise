#include "igCanvas.h"
#include "igShape.h"
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

igCanvas::igCanvas(const igVec2& size)
  : m_dimensions(size)
{
  m_data.resize(size.x * size.y); // by default a black canvas
}

igVec2 igCanvas::Size() const
{
  return m_dimensions;
}

bool igCanvas::Empty() const
{
  return m_data.empty();
}

igCanvas igCanvas::LoadImageCanvas(const std::string& path)
{
  cv::Mat img = cv::imread(path, cv::IMREAD_COLOR);
  if (img.empty())
  {
    return igCanvas();
  }

  igCanvas newCanvas(igVec2(img.cols, img.rows));

  for (int y = 0; y < img.rows; y++)
  {
    const cv::Vec3b* row = img.ptr<cv::Vec3b>(y);
    for (int x = 0; x < img.cols; x++)
    {
      newCanvas.SetPixel(x, y, igColour(row[x][2], row[x][1], row[x][0]));
    }
  }

  return newCanvas;
}

bool igCanvas::Contains(int64_t x, int64_t y) const
{
  return x >= 0 && y >= 0 && x < m_dimensions.x && y < m_dimensions.y;
}

igColour igCanvas::GetPixel(int64_t x, int64_t y) const
{
  return m_data[x + y * m_dimensions.x];
}

void igCanvas::SetPixel(int64_t x, int64_t y, const igColour& colour)
{
  m_data[x + y * m_dimensions.x] = colour;
}

void igCanvas::Project(const igShape& shape)
{
  const igColour colour = shape.Colour();
  shape.ForEachPixel([&](int64_t x, int64_t y)
                     {
    if (Contains(x, y))
    {
      SetPixel(x, y, colour.BlendOver(GetPixel(x, y)));
    } });
}

igCanvas igCanvas::Resized(const igVec2& size) const
{
  // to cv::Mat, resize, and back
  cv::Mat img(int(m_dimensions.y), int(m_dimensions.x), CV_8UC3);
  for (int y = 0; y < img.rows; y++)
  {
    cv::Vec3b* row = img.ptr<cv::Vec3b>(y);
    for (int x = 0; x < img.cols; x++)
    {
      const igColour c = GetPixel(x, y);
      row[x] = cv::Vec3b(c.blue, c.green, c.red);
    }
  }

  cv::Mat resized;
  cv::resize(img, resized, cv::Size(int(size.x), int(size.y)));

  igCanvas newCanvas(size);
  for (int y = 0; y < resized.rows; y++)
  {
    const cv::Vec3b* row = resized.ptr<cv::Vec3b>(y);
    for (int x = 0; x < resized.cols; x++)
    {
      newCanvas.SetPixel(x, y, igColour(row[x][2], row[x][1], row[x][0]));
    }
  }
  return newCanvas;
}
