cmake_minimum_required(VERSION 3.10)
project(PathTracing)

set(CMAKE_CXX_STANDARD 17)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/HW7.h.in" "${CMAKE_CURRENT_SOURCE_DIR}/HW7.h")
add_executable(PathTracing main.cpp Object.hpp Vector.cpp Vector.hpp Sphere.hpp global.hpp Triangle.hpp Scene.cpp
        Scene.hpp Light.hpp AreaLight.hpp BVH.cpp BVH.hpp Bounds3.hpp Ray.hpp Material.hpp Intersection.hpp
        Renderer.cpp Renderer.hpp)
