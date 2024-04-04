cmake_minimum_required(VERSION 3.10)
project(Rasterizer)

include("${CONFIG_PATH}/EigenInclude.cmake")
include("${CONFIG_PATH}/OpenCVInclude.cmake")

include_directories(/usr/local/include ./include)

add_executable(RasterizerHW3 main.cpp rasterizer.hpp rasterizer.cpp global.hpp Triangle.hpp Triangle.cpp Texture.hpp Texture.cpp Shader.hpp OBJ_Loader.h)
target_link_libraries(RasterizerHW3 ${OpenCV_LIBRARIES})
#target_compile_options(Rasterizer PUBLIC -Wall -Wextra -pedantic)