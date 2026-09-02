#include "imgproc/Processing.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>

namespace imgproc {
namespace {

Image::Pixel toPixel(double value) {
    return static_cast<Image::Pixel>(std::clamp(std::lround(value), 0L, 255L));
}

}  // namespace

BrightnessContrastAdjustment::BrightnessContrastAdjustment(double gain, double bias)
    : gain_(gain), bias_(bias) {
    if (!std::isfinite(gain) || !std::isfinite(bias)) {
        throw std::invalid_argument("Gain and bias must be finite");
    }
}

Image BrightnessContrastAdjustment::process(const Image& source) const {
    Image result(source.width(), source.height());
    for (std::size_t y = 0; y < source.height(); ++y) {
        for (std::size_t x = 0; x < source.width(); ++x) {
            result.at(x, y) = toPixel(gain_ * source.at(x, y) + bias_);
        }
    }
    return result;
}

GammaCorrection::GammaCorrection(double gamma) : gamma_(gamma) {
    if (!std::isfinite(gamma) || gamma <= 0.0) {
        throw std::invalid_argument("Gamma must be finite and greater than zero");
    }
}

Image GammaCorrection::process(const Image& source) const {
    Image result(source.width(), source.height());
    for (std::size_t y = 0; y < source.height(); ++y) {
        for (std::size_t x = 0; x < source.width(); ++x) {
            const double normalized = static_cast<double>(source.at(x, y)) / 255.0;
            result.at(x, y) = toPixel(std::pow(normalized, gamma_) * 255.0);
        }
    }
    return result;
}

Convolution::Convolution(std::vector<double> kernel, std::size_t kernelWidth,
                         std::size_t kernelHeight, BorderMode borderMode, double scale,
                         double bias, bool absoluteResponse)
    : kernel_(std::move(kernel)),
      kernelWidth_(kernelWidth),
      kernelHeight_(kernelHeight),
      borderMode_(borderMode),
      scale_(scale),
      bias_(bias),
      absoluteResponse_(absoluteResponse) {
    if (kernelWidth == 0 || kernelHeight == 0 || kernelWidth % 2 == 0 || kernelHeight % 2 == 0 ||
        kernel_.size() != kernelWidth * kernelHeight) {
        throw std::invalid_argument("Convolution kernels must have matching odd dimensions");
    }
    if (!std::isfinite(scale) || !std::isfinite(bias) ||
        !std::all_of(kernel_.begin(), kernel_.end(),
                     [](double value) { return std::isfinite(value); })) {
        throw std::invalid_argument("Convolution parameters must be finite");
    }
}

double Convolution::sample(const Image& source, int x, int y) const {
    if (x >= 0 && y >= 0 && x < static_cast<int>(source.width()) &&
        y < static_cast<int>(source.height())) {
        return source.at(static_cast<std::size_t>(x), static_cast<std::size_t>(y));
    }
    if (borderMode_ == BorderMode::Zero || source.empty()) {
        return 0.0;
    }

    const int clampedX = std::clamp(x, 0, static_cast<int>(source.width()) - 1);
    const int clampedY = std::clamp(y, 0, static_cast<int>(source.height()) - 1);
    return source.at(static_cast<std::size_t>(clampedX), static_cast<std::size_t>(clampedY));
}

Image Convolution::process(const Image& source) const {
    Image result(source.width(), source.height());
    if (source.empty()) {
        return result;
    }

    const int halfWidth = static_cast<int>(kernelWidth_ / 2);
    const int halfHeight = static_cast<int>(kernelHeight_ / 2);
    for (std::size_t y = 0; y < source.height(); ++y) {
        for (std::size_t x = 0; x < source.width(); ++x) {
            double response = 0.0;
            for (std::size_t ky = 0; ky < kernelHeight_; ++ky) {
                for (std::size_t kx = 0; kx < kernelWidth_; ++kx) {
                    const int sampleX = static_cast<int>(x) + static_cast<int>(kx) - halfWidth;
                    const int sampleY = static_cast<int>(y) + static_cast<int>(ky) - halfHeight;
                    response += sample(source, sampleX, sampleY) * kernel_[ky * kernelWidth_ + kx];
                }
            }
            response *= scale_;
            if (absoluteResponse_) {
                response = std::abs(response);
            }
            result.at(x, y) = toPixel(response + bias_);
        }
    }
    return result;
}

Convolution Convolution::meanBlur(BorderMode mode) {
    return Convolution({1, 1, 1, 1, 1, 1, 1, 1, 1}, 3, 3, mode, 1.0 / 9.0);
}

Convolution Convolution::gaussianBlur(BorderMode mode) {
    return Convolution({1, 2, 1, 2, 4, 2, 1, 2, 1}, 3, 3, mode, 1.0 / 16.0);
}

Convolution Convolution::sobelX(BorderMode mode) {
    return Convolution({-1, 0, 1, -2, 0, 2, -1, 0, 1}, 3, 3, mode, 1.0, 0.0, true);
}

Convolution Convolution::sobelY(BorderMode mode) {
    return Convolution({-1, -2, -1, 0, 0, 0, 1, 2, 1}, 3, 3, mode, 1.0, 0.0, true);
}

}  // namespace imgproc
