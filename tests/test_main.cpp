#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "imgproc/Draw.h"
#include "imgproc/Image.h"
#include "imgproc/Processing.h"

namespace {

int failures = 0;

#define CHECK(condition)                                                                  \
    do {                                                                                  \
        if (!(condition)) {                                                               \
            std::cerr << "  check failed at " << __FILE__ << ':' << __LINE__ << ": "    \
                      << #condition << '\n';                                              \
            ++failures;                                                                   \
        }                                                                                 \
    } while (false)

template <typename Exception, typename Function>
void checkThrows(Function&& function) {
    bool threwExpected = false;
    try {
        function();
    } catch (const Exception&) {
        threwExpected = true;
    } catch (...) {
    }
    CHECK(threwExpected);
}

void testRuleOfFive() {
    imgproc::Image original(3, 2, 10);
    original.at(1, 1) = 90;

    imgproc::Image copy = original;
    copy.at(1, 1) = 5;
    CHECK(original.at(1, 1) == 90);

    imgproc::Image assigned(1, 1, 0);
    assigned = original;
    CHECK(assigned.width() == 3);
    CHECK(assigned.height() == 2);
    CHECK(assigned.at(1, 1) == 90);

    imgproc::Image moved = std::move(assigned);
    CHECK(moved.at(1, 1) == 90);
    CHECK(assigned.empty());
}

void testArithmeticAndBounds() {
    imgproc::Image left(2, 1);
    imgproc::Image right(2, 1);
    left.at(0, 0) = 250;
    left.at(1, 0) = 10;
    right.at(0, 0) = 20;
    right.at(1, 0) = 40;

    CHECK((left + right).at(0, 0) == 255);
    CHECK((left - right).at(1, 0) == 0);
    CHECK((right * 2.0).at(1, 0) == 80);
    checkThrows<std::out_of_range>([&] { static_cast<void>(left.at(2, 0)); });
    checkThrows<std::invalid_argument>([&] { static_cast<void>(left + imgproc::Image(1, 1)); });
}

void testRoiAndGeometry() {
    imgproc::Image image(4, 3);
    for (std::size_t y = 0; y < image.height(); ++y) {
        for (std::size_t x = 0; x < image.width(); ++x) {
            image.at(x, y) = static_cast<imgproc::Image::Pixel>(y * 10 + x);
        }
    }

    const imgproc::Image region = image.roi({1, 1, 2, 2});
    CHECK(region.width() == 2);
    CHECK(region.height() == 2);
    CHECK(region.at(0, 0) == 11);
    CHECK(region.at(1, 1) == 22);
    checkThrows<std::out_of_range>([&] { static_cast<void>(image.roi({3, 2, 2, 2})); });

    const imgproc::Rectangle overlap = imgproc::Rectangle{0, 0, 4, 3}.intersection({2, 1, 4, 3});
    CHECK(overlap.x == 2 && overlap.y == 1 && overlap.width == 2 && overlap.height == 2);
}

void testPgmRoundTrip() {
    const std::filesystem::path directory =
        std::filesystem::temp_directory_path() / "cpp-image-processing-tests";
    std::filesystem::create_directories(directory);

    imgproc::Image source(3, 2);
    source.at(0, 0) = 0;
    source.at(1, 0) = 64;
    source.at(2, 0) = 128;
    source.at(0, 1) = 192;
    source.at(1, 1) = 224;
    source.at(2, 1) = 255;

    for (const auto format : {imgproc::PgmFormat::Ascii, imgproc::PgmFormat::Binary}) {
        const auto path = directory / (format == imgproc::PgmFormat::Ascii ? "roundtrip-p2.pgm"
                                                                           : "roundtrip-p5.pgm");
        source.savePgm(path, format);
        const imgproc::Image loaded = imgproc::Image::loadPgm(path);
        CHECK(loaded.width() == source.width());
        CHECK(loaded.height() == source.height());
        for (std::size_t y = 0; y < source.height(); ++y) {
            for (std::size_t x = 0; x < source.width(); ++x) {
                CHECK(loaded.at(x, y) == source.at(x, y));
            }
        }
        std::filesystem::remove(path);
    }
    std::filesystem::remove(directory);
}

void testPointProcessors() {
    imgproc::Image source(2, 1);
    source.at(0, 0) = 0;
    source.at(1, 0) = 100;

    std::unique_ptr<imgproc::ImageProcessor> processor =
        std::make_unique<imgproc::BrightnessContrastAdjustment>(2.0, 10.0);
    const imgproc::Image adjusted = processor->process(source);
    CHECK(adjusted.at(0, 0) == 10);
    CHECK(adjusted.at(1, 0) == 210);

    imgproc::GammaCorrection identity(1.0);
    CHECK(identity.process(source).at(1, 0) == 100);
    checkThrows<std::invalid_argument>([] { imgproc::GammaCorrection invalid(0.0); });
}

void testConvolution() {
    imgproc::Image constant(5, 5, 90);
    const imgproc::Image blurred = imgproc::Convolution::gaussianBlur().process(constant);
    CHECK(blurred.at(0, 0) == 90);
    CHECK(blurred.at(2, 2) == 90);

    const imgproc::Image edges = imgproc::Convolution::sobelX().process(constant);
    CHECK(edges.at(2, 2) == 0);

    imgproc::Image impulse(3, 3, 0);
    impulse.at(1, 1) = 255;
    const imgproc::Image mean = imgproc::Convolution::meanBlur(imgproc::BorderMode::Zero).process(impulse);
    CHECK(mean.at(1, 1) == 28);
}

void testDrawing() {
    imgproc::Image canvas(11, 11, 0);
    imgproc::Draw::line(canvas, {0, 0}, {10, 10}, 100);
    CHECK(canvas.at(0, 0) == 100);
    CHECK(canvas.at(10, 10) == 100);

    imgproc::Draw::rectangle(canvas, {2, 2, 5, 4}, 150);
    CHECK(canvas.at(2, 2) == 150);
    CHECK(canvas.at(6, 5) == 150);

    imgproc::Draw::circle(canvas, {5, 5}, 3, 200);
    CHECK(canvas.at(8, 5) == 200);
    CHECK(canvas.at(5, 2) == 200);
    checkThrows<std::invalid_argument>([&] { imgproc::Draw::circle(canvas, {5, 5}, -1, 0); });
}

}

int main() {
    const std::vector<std::pair<std::string, std::function<void()>>> tests = {
        {"Rule of Five", testRuleOfFive},
        {"arithmetic and bounds", testArithmeticAndBounds},
        {"ROI and geometry", testRoiAndGeometry},
        {"PGM round trip", testPgmRoundTrip},
        {"point processors", testPointProcessors},
        {"convolution", testConvolution},
        {"drawing", testDrawing},
    };

    for (const auto& [name, test] : tests) {
        const int before = failures;
        try {
            test();
        } catch (const std::exception& error) {
            std::cerr << "  unexpected exception in " << name << ": " << error.what() << '\n';
            ++failures;
        }
        std::cout << (failures == before ? "[pass] " : "[fail] ") << name << '\n';
    }

    if (failures != 0) {
        std::cerr << failures << " check(s) failed\n";
        return 1;
    }
    std::cout << "All " << tests.size() << " tests passed\n";
    return 0;
}
