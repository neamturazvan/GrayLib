#include "imgproc/Draw.h"

#include <algorithm>
#include <cstdlib>
#include <stdexcept>

namespace imgproc {

void Draw::setIfInside(Image& image, int x, int y, Image::Pixel color) {
    if (x >= 0 && y >= 0 && x < static_cast<int>(image.width()) &&
        y < static_cast<int>(image.height())) {
        image.at(static_cast<std::size_t>(x), static_cast<std::size_t>(y)) = color;
    }
}

void Draw::line(Image& image, Point start, Point end, Image::Pixel color) {
    int x = start.x;
    int y = start.y;
    const int dx = std::abs(end.x - start.x);
    const int sx = start.x < end.x ? 1 : -1;
    const int dy = -std::abs(end.y - start.y);
    const int sy = start.y < end.y ? 1 : -1;
    int error = dx + dy;

    while (true) {
        setIfInside(image, x, y, color);
        if (x == end.x && y == end.y) {
            break;
        }
        const int doubled = 2 * error;
        if (doubled >= dy) {
            error += dy;
            x += sx;
        }
        if (doubled <= dx) {
            error += dx;
            y += sy;
        }
    }
}

void Draw::rectangle(Image& image, Rectangle bounds, Image::Pixel color) {
    if (bounds.empty()) {
        return;
    }
    const int right = bounds.x + static_cast<int>(bounds.width) - 1;
    const int bottom = bounds.y + static_cast<int>(bounds.height) - 1;
    rectangle(image, {bounds.x, bounds.y}, {right, bottom}, color);
}

void Draw::rectangle(Image& image, Point topLeft, Point bottomRight, Image::Pixel color) {
    const int left = std::min(topLeft.x, bottomRight.x);
    const int right = std::max(topLeft.x, bottomRight.x);
    const int top = std::min(topLeft.y, bottomRight.y);
    const int bottom = std::max(topLeft.y, bottomRight.y);

    line(image, {left, top}, {right, top}, color);
    line(image, {right, top}, {right, bottom}, color);
    line(image, {right, bottom}, {left, bottom}, color);
    line(image, {left, bottom}, {left, top}, color);
}

void Draw::circle(Image& image, Point center, int radius, Image::Pixel color) {
    if (radius < 0) {
        throw std::invalid_argument("Circle radius cannot be negative");
    }

    int x = radius;
    int y = 0;
    int error = 1 - radius;
    while (x >= y) {
        setIfInside(image, center.x + x, center.y + y, color);
        setIfInside(image, center.x + y, center.y + x, color);
        setIfInside(image, center.x - y, center.y + x, color);
        setIfInside(image, center.x - x, center.y + y, color);
        setIfInside(image, center.x - x, center.y - y, color);
        setIfInside(image, center.x - y, center.y - x, color);
        setIfInside(image, center.x + y, center.y - x, color);
        setIfInside(image, center.x + x, center.y - y, color);

        ++y;
        if (error < 0) {
            error += 2 * y + 1;
        } else {
            --x;
            error += 2 * (y - x) + 1;
        }
    }
}

}  // namespace imgproc
