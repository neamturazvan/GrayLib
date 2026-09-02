#ifndef IMGPROC_PROCESSING_H
#define IMGPROC_PROCESSING_H

#include <cstddef>
#include <vector>

#include "imgproc/Image.h"

namespace imgproc {

enum class BorderMode { Zero, Extend };

class ImageProcessor {
public:
    virtual ~ImageProcessor() = default;
    [[nodiscard]] virtual Image process(const Image& source) const = 0;
};

class BrightnessContrastAdjustment final : public ImageProcessor {
public:
    BrightnessContrastAdjustment(double gain = 1.0, double bias = 0.0);
    [[nodiscard]] Image process(const Image& source) const override;

private:
    double gain_;
    double bias_;
};

class GammaCorrection final : public ImageProcessor {
public:
    explicit GammaCorrection(double gamma = 1.0);
    [[nodiscard]] Image process(const Image& source) const override;

private:
    double gamma_;
};

class Convolution final : public ImageProcessor {
public:
    Convolution(std::vector<double> kernel, std::size_t kernelWidth,
                std::size_t kernelHeight, BorderMode borderMode = BorderMode::Extend,
                double scale = 1.0, double bias = 0.0, bool absoluteResponse = false);

    [[nodiscard]] Image process(const Image& source) const override;

    [[nodiscard]] static Convolution meanBlur(BorderMode mode = BorderMode::Extend);
    [[nodiscard]] static Convolution gaussianBlur(BorderMode mode = BorderMode::Extend);
    [[nodiscard]] static Convolution sobelX(BorderMode mode = BorderMode::Extend);
    [[nodiscard]] static Convolution sobelY(BorderMode mode = BorderMode::Extend);

private:
    std::vector<double> kernel_;
    std::size_t kernelWidth_;
    std::size_t kernelHeight_;
    BorderMode borderMode_;
    double scale_;
    double bias_;
    bool absoluteResponse_;

    [[nodiscard]] double sample(const Image& source, int x, int y) const;
};

}  // namespace imgproc

#endif
