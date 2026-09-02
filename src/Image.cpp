#include "imgproc/Image.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>

namespace imgproc {
namespace {

std::string nextToken(std::istream& input) {
    while (true) {
        input >> std::ws;
        if (!input) {
            throw std::runtime_error("Unexpected end of PGM file");
        }
        if (input.peek() == '#') {
            input.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }

        std::string token;
        input >> token;
        if (!token.empty()) {
            return token;
        }
    }
}

std::size_t parseSize(const std::string& token, const char* field) {
    std::size_t consumed = 0;
    unsigned long long value = 0;
    try {
        value = std::stoull(token, &consumed);
    } catch (const std::exception&) {
        throw std::runtime_error(std::string("Invalid PGM ") + field);
    }
    if (consumed != token.size() || value == 0 || value > std::numeric_limits<std::size_t>::max()) {
        throw std::runtime_error(std::string("Invalid PGM ") + field);
    }
    return static_cast<std::size_t>(value);
}

int parsePixel(const std::string& token, int maximum) {
    std::size_t consumed = 0;
    int value = 0;
    try {
        value = std::stoi(token, &consumed);
    } catch (const std::exception&) {
        throw std::runtime_error("Invalid PGM pixel value");
    }
    if (consumed != token.size() || value < 0 || value > maximum) {
        throw std::runtime_error("PGM pixel value is outside the declared range");
    }
    return value;
}

Image::Pixel clampPixel(double value) {
    if (!std::isfinite(value)) {
        throw std::invalid_argument("Pixel calculation produced a non-finite value");
    }
    return static_cast<Image::Pixel>(std::clamp(std::lround(value), 0L, 255L));
}

}  // namespace

Image::Image(std::size_t width, std::size_t height, Pixel fill)
    : width_(width), height_(height) {
    if (width == 0 || height == 0) {
        if (width != 0 || height != 0) {
            throw std::invalid_argument("Image dimensions must both be zero or both be positive");
        }
        return;
    }
    if (width > std::numeric_limits<std::size_t>::max() / height) {
        throw std::length_error("Image dimensions are too large");
    }

    data_ = new Pixel[width * height];
    std::fill(data_, data_ + width * height, fill);
}

Image::Image(const Image& other) : Image(other.width_, other.height_) {
    if (data_ != nullptr) {
        std::copy(other.data_, other.data_ + width_ * height_, data_);
    }
}

Image::Image(Image&& other) noexcept
    : data_(std::exchange(other.data_, nullptr)),
      width_(std::exchange(other.width_, 0)),
      height_(std::exchange(other.height_, 0)) {}

Image& Image::operator=(const Image& other) {
    if (this != &other) {
        Image copy(other);
        swap(copy);
    }
    return *this;
}

Image& Image::operator=(Image&& other) noexcept {
    if (this != &other) {
        delete[] data_;
        data_ = std::exchange(other.data_, nullptr);
        width_ = std::exchange(other.width_, 0);
        height_ = std::exchange(other.height_, 0);
    }
    return *this;
}

Image::~Image() { delete[] data_; }

void Image::swap(Image& other) noexcept {
    std::swap(data_, other.data_);
    std::swap(width_, other.width_);
    std::swap(height_, other.height_);
}

std::size_t Image::index(std::size_t x, std::size_t y) const {
    if (x >= width_ || y >= height_) {
        throw std::out_of_range("Pixel coordinates are outside the image");
    }
    return y * width_ + x;
}

Image::Pixel& Image::at(std::size_t x, std::size_t y) { return data_[index(x, y)]; }

const Image::Pixel& Image::at(std::size_t x, std::size_t y) const {
    return data_[index(x, y)];
}

Image::Pixel* Image::row(std::size_t y) {
    if (y >= height_) {
        throw std::out_of_range("Row is outside the image");
    }
    return data_ + y * width_;
}

const Image::Pixel* Image::row(std::size_t y) const {
    if (y >= height_) {
        throw std::out_of_range("Row is outside the image");
    }
    return data_ + y * width_;
}

Image Image::loadPgm(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        throw std::runtime_error("Could not open PGM file: " + path.string());
    }

    const std::string magic = nextToken(input);
    if (magic != "P2" && magic != "P5") {
        throw std::runtime_error("Unsupported PGM format; expected P2 or P5");
    }

    const std::size_t width = parseSize(nextToken(input), "width");
    const std::size_t height = parseSize(nextToken(input), "height");
    const int maximum = parsePixel(nextToken(input), 65535);
    if (maximum == 0 || maximum > 255) {
        throw std::runtime_error("Only 8-bit PGM files are supported");
    }

    Image image(width, height);
    const std::size_t count = width * height;

    if (magic == "P2") {
        for (std::size_t i = 0; i < count; ++i) {
            const int value = parsePixel(nextToken(input), maximum);
            image.data_[i] = static_cast<Pixel>((value * 255 + maximum / 2) / maximum);
        }
    } else {
        char separator = '\0';
        input.get(separator);
        if (!input || !std::isspace(static_cast<unsigned char>(separator))) {
            throw std::runtime_error("Invalid P5 header separator");
        }
        if (separator == '\r' && input.peek() == '\n') {
            input.get();
        }
        input.read(reinterpret_cast<char*>(image.data_), static_cast<std::streamsize>(count));
        if (input.gcount() != static_cast<std::streamsize>(count)) {
            throw std::runtime_error("P5 pixel data is truncated");
        }
        if (maximum != 255) {
            for (std::size_t i = 0; i < count; ++i) {
                image.data_[i] = static_cast<Pixel>((image.data_[i] * 255 + maximum / 2) / maximum);
            }
        }
    }
    return image;
}

void Image::savePgm(const std::filesystem::path& path, PgmFormat format) const {
    if (empty()) {
        throw std::logic_error("Cannot save an empty image");
    }

    std::ofstream output(path, std::ios::binary);
    if (!output) {
        throw std::runtime_error("Could not create PGM file: " + path.string());
    }

    if (format == PgmFormat::Binary) {
        output << "P5\n" << width_ << ' ' << height_ << "\n255\n";
        output.write(reinterpret_cast<const char*>(data_),
                     static_cast<std::streamsize>(width_ * height_));
    } else {
        output << "P2\n# Generated by cpp-image-processing\n" << width_ << ' ' << height_
               << "\n255\n";
        for (std::size_t y = 0; y < height_; ++y) {
            for (std::size_t x = 0; x < width_; ++x) {
                output << static_cast<int>(at(x, y));
                output << (x + 1 == width_ ? '\n' : ' ');
            }
        }
    }

    if (!output) {
        throw std::runtime_error("Failed while writing PGM file: " + path.string());
    }
}

Image Image::roi(Rectangle region) const {
    if (region.x < 0 || region.y < 0 || region.empty() ||
        static_cast<std::size_t>(region.x) + region.width > width_ ||
        static_cast<std::size_t>(region.y) + region.height > height_) {
        throw std::out_of_range("ROI is outside the image");
    }

    Image result(region.width, region.height);
    for (std::size_t y = 0; y < region.height; ++y) {
        for (std::size_t x = 0; x < region.width; ++x) {
            result.at(x, y) = at(static_cast<std::size_t>(region.x) + x,
                                 static_cast<std::size_t>(region.y) + y);
        }
    }
    return result;
}

void Image::requireSameSize(const Image& other) const {
    if (size().width != other.size().width || size().height != other.size().height) {
        throw std::invalid_argument("Image dimensions do not match");
    }
}

Image Image::operator+(const Image& other) const {
    requireSameSize(other);
    Image result(width_, height_);
    for (std::size_t i = 0; i < width_ * height_; ++i) {
        result.data_[i] = clampPixel(static_cast<int>(data_[i]) + other.data_[i]);
    }
    return result;
}

Image Image::operator-(const Image& other) const {
    requireSameSize(other);
    Image result(width_, height_);
    for (std::size_t i = 0; i < width_ * height_; ++i) {
        result.data_[i] = clampPixel(static_cast<int>(data_[i]) - other.data_[i]);
    }
    return result;
}

Image Image::operator*(double scalar) const {
    if (!std::isfinite(scalar)) {
        throw std::invalid_argument("Scalar must be finite");
    }
    Image result(width_, height_);
    for (std::size_t i = 0; i < width_ * height_; ++i) {
        result.data_[i] = clampPixel(static_cast<double>(data_[i]) * scalar);
    }
    return result;
}

Image Image::zeros(std::size_t width, std::size_t height) { return Image(width, height, 0); }

Image Image::ones(std::size_t width, std::size_t height) { return Image(width, height, 1); }

}  // namespace imgproc
