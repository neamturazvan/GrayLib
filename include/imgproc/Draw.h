#ifndef IMGPROC_DRAW_H
#define IMGPROC_DRAW_H

#include "imgproc/Geometry.h"
#include "imgproc/Image.h"

namespace imgproc {

class Draw {
public:
    static void line(Image& image, Point start, Point end, Image::Pixel color);
    static void rectangle(Image& image, Rectangle bounds, Image::Pixel color);
    static void rectangle(Image& image, Point topLeft, Point bottomRight, Image::Pixel color);
    static void circle(Image& image, Point center, int radius, Image::Pixel color);

private:
    static void setIfInside(Image& image, int x, int y, Image::Pixel color);
};

}  // namespace imgproc

#endif
