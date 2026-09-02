#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iostream>

#include "imgproc/Draw.h"
#include "imgproc/Image.h"
#include "imgproc/Processing.h"

namespace {

imgproc::Image makeInput(std::size_t width, std::size_t height) {
    imgproc::Image image(width, height);
    for (std::size_t y = 0; y < height; ++y) {
        for (std::size_t x = 0; x < width; ++x) {
            const double gradient = 30.0 + 150.0 * static_cast<double>(x) / (width - 1);
            const double wave = 30.0 * std::sin(static_cast<double>(y) / 13.0);
            const int checker = ((x / 24 + y / 24) % 2 == 0) ? 18 : -18;
            image.at(x, y) = static_cast<imgproc::Image::Pixel>(
                std::clamp(std::lround(gradient + wave + checker), 0L, 255L));
        }
    }
    return image;
}

}  // namespace

int main(int argc, char* argv[]) {
    try {
        const std::filesystem::path outputDirectory = argc > 1 ? argv[1] : "demo-output";
        std::filesystem::create_directories(outputDirectory);

        const imgproc::Image input = makeInput(256, 192);
        input.savePgm(outputDirectory / "input.pgm", imgproc::PgmFormat::Binary);

        imgproc::BrightnessContrastAdjustment contrast(1.25, 12.0);
        contrast.process(input).savePgm(outputDirectory / "contrast.pgm",
                                        imgproc::PgmFormat::Binary);

        imgproc::GammaCorrection gamma(0.6);
        gamma.process(input).savePgm(outputDirectory / "gamma.pgm", imgproc::PgmFormat::Binary);

        imgproc::Convolution::gaussianBlur().process(input).savePgm(
            outputDirectory / "gaussian-blur.pgm", imgproc::PgmFormat::Binary);
        imgproc::Convolution::sobelX().process(input).savePgm(outputDirectory / "sobel-x.pgm",
                                                               imgproc::PgmFormat::Binary);

        imgproc::Image drawing = input;
        imgproc::Draw::rectangle(drawing, {20, 18, 96, 72}, 255);
        imgproc::Draw::line(drawing, {15, 170}, {235, 35}, 240);
        imgproc::Draw::circle(drawing, {185, 120}, 42, 255);
        drawing.savePgm(outputDirectory / "drawing.pgm", imgproc::PgmFormat::Binary);

        input.roi({72, 48, 112, 96})
            .savePgm(outputDirectory / "roi.pgm", imgproc::PgmFormat::Binary);

        std::cout << "Wrote demo images to " << outputDirectory << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Demo failed: " << error.what() << '\n';
        return 1;
    }
}
