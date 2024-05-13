//
// Created by goksu on 2/25/20.
//

#include "Renderer.hpp"
#include "HW7.h"
#include "Scene.hpp"
#include "global.hpp"
#include <atomic>
#include <filesystem>
#include <fstream>
#include <thread>
#include <vector>

inline float deg2rad(const float &deg) { return deg * M_PI / 180.0; }

const float EPSILON = 0.00001;
void RenderMultiThread(const Scene &scene) {}
// The main render function. This where we iterate over all pixels in the image,
// generate primary rays and cast these rays into the scene. The content of the
// framebuffer is saved to a file.
void Renderer::Render(const Scene &scene) {
  std::vector<Vector3f> framebuffer(scene.width * scene.height);

  float scale = tan(deg2rad(scene.fov * 0.5));
  float imageAspectRatio = scene.width / (float)scene.height;
  Vector3f eye_pos(278, 273, -800);
  // int m = 0;

  // change the spp value to change sample ammount
  int spp = global_args.spp;
  std::cout << "SPP: " << spp << "\n";
  std::atomic_int finish_count{0};

  auto render_part = [&](int s, int e) {
    for (uint32_t j = s; j < e; ++j) {
      for (uint32_t i = 0; i < scene.width; ++i) {
        // generate primary ray direction
        float x =
            (2 * (i + 0.5) / (float)scene.width - 1) * imageAspectRatio * scale;
        float y = (1 - 2 * (j + 0.5) / (float)scene.height) * scale;

        Vector3f dir = normalize(Vector3f(-x, y, 1));
        for (int k = 0; k < spp; k++) {
          framebuffer[j * scene.height + i] +=
              scene.castRay(Ray(eye_pos, dir), 0) / spp;
        }
      }
      finish_count.fetch_add(1);
      UpdateProgress(finish_count / (float)scene.height);
    }
  };

  if (global_args.thread_count == 1) {
    render_part(0, scene.height);
  } else {
    int group = scene.height / global_args.thread_count;
    int end_group = scene.height % global_args.thread_count;
    std::vector<std::thread> ths;
    for (int i = 0; i < global_args.thread_count; i++) {
      ths.push_back(std::thread{render_part, i * group, (i + 1) * group});
    }
    if (end_group != 0) {
      ths.push_back(std::thread{render_part, global_args.thread_count * group,
                                global_args.thread_count * group + end_group});
    }

    for (auto &th : ths) {
      th.join();
    }
  }
  UpdateProgress(1.f);

  // save framebuffer to file
  std::filesystem::path save_path =
      std::string(OUT_DIR) + global_args.save_name;
  FILE *fp = fopen(save_path.c_str(), "wb");
  (void)fprintf(fp, "P6\n%d %d\n255\n", scene.width, scene.height);
  for (auto i = 0; i < scene.height * scene.width; ++i) {
    static unsigned char color[3];
    color[0] =
        (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].x), 0.6f));
    color[1] =
        (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].y), 0.6f));
    color[2] =
        (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].z), 0.6f));
    fwrite(color, 1, 3, fp);
  }
  fclose(fp);
}
