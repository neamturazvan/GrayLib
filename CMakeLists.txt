cmake_minimum_required(VERSION 3.20)

project(cpp_image_processing VERSION 1.0.0 LANGUAGES CXX)

option(IMAGE_PROCESSING_BUILD_EXAMPLES "Build the example program" ON)
option(IMAGE_PROCESSING_BUILD_TESTS "Build the test suite" ON)

add_library(image_processing
    src/Draw.cpp
    src/Image.cpp
    src/Processing.cpp
)

add_library(image_processing::image_processing ALIAS image_processing)

target_compile_features(image_processing PUBLIC cxx_std_17)
target_include_directories(image_processing
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

if(MSVC)
    target_compile_options(image_processing PRIVATE /W4 /permissive-)
else()
    target_compile_options(image_processing PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(IMAGE_PROCESSING_BUILD_EXAMPLES)
    add_executable(image_processing_demo examples/demo.cpp)
    target_include_directories(image_processing_demo PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(image_processing_demo PRIVATE image_processing::image_processing)
endif()

if(IMAGE_PROCESSING_BUILD_TESTS)
    include(CTest)
    add_executable(image_processing_tests tests/test_main.cpp)
    target_link_libraries(image_processing_tests PRIVATE image_processing::image_processing)
    add_test(NAME image_processing_tests COMMAND image_processing_tests)
endif()
