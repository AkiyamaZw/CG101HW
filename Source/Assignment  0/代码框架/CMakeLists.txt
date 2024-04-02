cmake_minimum_required (VERSION 3.6)
project (Transformation)

include("${CONFIG_PATH}/EigenInclude.cmake")
# find_package(Eigen3 REQUIRED)
# include_directories(EIGEN3_INCLUDE_DIR)

add_executable (Transformation main.cpp)
