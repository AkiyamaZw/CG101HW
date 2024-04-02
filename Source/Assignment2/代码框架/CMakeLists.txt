cmake_minimum_required(VERSION 3.10)
project(Rasterizer)

include("${CONFIG_PATH}/EigenInclude.cmake")
include("${CONFIG_PATH}/OpenCVInclude.cmake")

add_executable(RasterizerHW2 main.cpp rasterizer.hpp rasterizer.cpp global.hpp Triangle.hpp Triangle.cpp)
target_link_libraries(RasterizerHW2 ${OpenCV_LIBRARIES})
