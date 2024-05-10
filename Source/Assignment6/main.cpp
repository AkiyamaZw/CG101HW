#include "BVH.hpp"
#include "HW6.h"
#include "Renderer.hpp"
#include "Scene.hpp"
#include "Triangle.hpp"
#include "Vector.hpp"
#include "global.hpp"
#include <chrono>
#include <filesystem>

// In the main function of the program, we create the scene (create objects and
// lights) as well as set the options for the render (image width and height,
// maximum recursion depth, field-of-view, etc.). We then call the render
// function().
int main(int argc, char **argv) {

  std::string build_bvh_method = "NAIVE";
  if (argc > 1) {
    build_bvh_method = std::string(argv[1]);
  }

  BVHAccel::SplitMethod global_split_method =
      build_bvh_method == "SAH" ? BVHAccel::SplitMethod::SAH
                                : BVHAccel::SplitMethod::NAIVE;
  Scene scene(1280, 960);
  std::filesystem::path model_path =
      std::string(SOURCE_DIR) + "/" + "models/bunny/bunny.obj";
  MeshTriangle bunny(model_path.c_str(), global_split_method);

  scene.Add(&bunny);
  scene.Add(std::make_unique<Light>(Vector3f(-20, 70, 20), 1));
  scene.Add(std::make_unique<Light>(Vector3f(20, 70, 20), 1));
  scene.buildBVH(global_split_method);

  Renderer r;

  auto start = std::chrono::system_clock::now();
  r.Render(scene);
  auto stop = std::chrono::system_clock::now();

  std::cout << "Render complete: \n";
  std::cout
      << "Time taken: "
      << std::chrono::duration_cast<std::chrono::hours>(stop - start).count()
      << " hours\n";
  std::cout
      << "          : "
      << std::chrono::duration_cast<std::chrono::minutes>(stop - start).count()
      << " minutes\n";
  std::cout
      << "          : "
      << std::chrono::duration_cast<std::chrono::seconds>(stop - start).count()
      << " seconds\n";
  std::cout << "          : "
            << std::chrono::duration_cast<std::chrono::milliseconds>(stop -
                                                                     start)
                   .count()
            << " milliseconds\n";

  return 0;
}