#ifndef IMGPROC_GEOMETRY_H
#define IMGPROC_GEOMETRY_H

#include <algorithm>
#include <cstddef>

namespace imgproc {

struct Point {
    int x{0};
    int y{0};
};

struct Size {
    std::size_t width{0};
    std::size_t height{0};

    [[nodiscard]] constexpr std::size_t area() const noexcept { return width * height; }
};

struct Rectangle {
    int x{0};
    int y{0};
    std::size_t width{0};
    std::size_t height{0};

    [[nodiscard]] constexpr bool empty() const noexcept { return width == 0 || height == 0; }

    [[nodiscard]] Rectangle translated(Point offset) const noexcept {
        return {x + offset.x, y + offset.y, width, height};
    }

    [[nodiscard]] Rectangle intersection(const Rectangle& other) const noexcept {
        const int left = std::max(x, other.x);
        const int top = std::max(y, other.y);
        const int right = std::min(x + static_cast<int>(width),
                                   other.x + static_cast<int>(other.width));
        const int bottom = std::min(y + static_cast<int>(height),
                                    other.y + static_cast<int>(other.height));

        if (right <= left || bottom <= top) {
            return {};
        }
        return {left, top, static_cast<std::size_t>(right - left),
                static_cast<std::size_t>(bottom - top)};
    }

    [[nodiscard]] Rectangle united(const Rectangle& other) const noexcept {
        if (empty()) {
            return other;
        }
        if (other.empty()) {
            return *this;
        }

        const int left = std::min(x, other.x);
        const int top = std::min(y, other.y);
        const int right = std::max(x + static_cast<int>(width),
                                   other.x + static_cast<int>(other.width));
        const int bottom = std::max(y + static_cast<int>(height),
                                    other.y + static_cast<int>(other.height));
        return {left, top, static_cast<std::size_t>(right - left),
                static_cast<std::size_t>(bottom - top)};
    }
};

}  // namespace imgproc

#endif
