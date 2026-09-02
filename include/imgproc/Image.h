#ifndef IMGPROC_IMAGE_H
#define IMGPROC_IMAGE_H

#include <cstddef>
#include <cstdint>
#include <filesystem>

#include "imgproc/Geometry.h"

namespace imgproc {

enum class PgmFormat { Ascii, Binary };

class Image {
public:
    using Pixel = std::uint8_t;

    Image() noexcept = default;
    Image(std::size_t width, std::size_t height, Pixel fill = 0);
    Image(const Image& other);
    Image(Image&& other) noexcept;
    Image& operator=(const Image& other);
    Image& operator=(Image&& other) noexcept;
    ~Image();

    void swap(Image& other) noexcept;

    [[nodiscard]] static Image loadPgm(const std::filesystem::path& path);
    void savePgm(const std::filesystem::path& path,
                 PgmFormat format = PgmFormat::Ascii) const;

    [[nodiscard]] std::size_t width() const noexcept { return width_; }
    [[nodiscard]] std::size_t height() const noexcept { return height_; }
    [[nodiscard]] Size size() const noexcept { return {width_, height_}; }
    [[nodiscard]] bool empty() const noexcept { return data_ == nullptr; }

    Pixel& at(std::size_t x, std::size_t y);
    const Pixel& at(std::size_t x, std::size_t y) const;
    Pixel* row(std::size_t y);
    const Pixel* row(std::size_t y) const;

    [[nodiscard]] Image roi(Rectangle region) const;

    [[nodiscard]] Image operator+(const Image& other) const;
    [[nodiscard]] Image operator-(const Image& other) const;
    [[nodiscard]] Image operator*(double scalar) const;

    [[nodiscard]] static Image zeros(std::size_t width, std::size_t height);
    [[nodiscard]] static Image ones(std::size_t width, std::size_t height);

private:
    Pixel* data_{nullptr};
    std::size_t width_{0};
    std::size_t height_{0};

    [[nodiscard]] std::size_t index(std::size_t x, std::size_t y) const;
    void requireSameSize(const Image& other) const;
};

}  // namespace imgproc

#endif
