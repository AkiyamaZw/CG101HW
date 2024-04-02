cmake_minimum_required(VERSION 3.10)
project(Rasterizer)

include("${CONFIG_PATH}/EigenInclude.cmake")
include("${CONFIG_PATH}/OpenCVInclude.cmake")


add_executable(Rasterizer main.cpp rasterizer.hpp rasterizer.cpp Triangle.hpp Triangle.cpp)
target_link_libraries(Rasterizer ${OpenCV_LIBRARIES})
