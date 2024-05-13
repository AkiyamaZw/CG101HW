#pragma once
#include <cmath>
#include <cstring>
#include <iostream>
#include <random>

#undef M_PI
#define M_PI 3.141592653589793f

extern const float EPSILON;
const float kInfinity = std::numeric_limits<float>::max();

inline float clamp(const float &lo, const float &hi, const float &v) {
  return std::max(lo, std::min(hi, v));
}

inline bool solveQuadratic(const float &a, const float &b, const float &c,
                           float &x0, float &x1) {
  float discr = b * b - 4 * a * c;
  if (discr < 0)
    return false;
  else if (discr == 0)
    x0 = x1 = -0.5 * b / a;
  else {
    float q = (b > 0) ? -0.5 * (b + sqrt(discr)) : -0.5 * (b - sqrt(discr));
    x0 = q / a;
    x1 = c / q;
  }
  if (x0 > x1)
    std::swap(x0, x1);
  return true;
}

inline float get_random_float() {
  std::random_device dev;
  std::mt19937 rng(dev());
  std::uniform_real_distribution<float> dist(
      0.f, 1.f); // distribution in range [1, 6]

  return dist(rng);
}

inline void UpdateProgress(float progress) {
  int barWidth = 70;

  std::cout << "[";
  int pos = barWidth * progress;
  for (int i = 0; i < barWidth; ++i) {
    if (i < pos)
      std::cout << "=";
    else if (i == pos)
      std::cout << ">";
    else
      std::cout << " ";
  }
  std::cout << "] " << int(progress * 100.0) << " %\r";
  std::cout.flush();
};

struct RuntimeArgs {
  // save width
  int width{784};
  // save height
  int height{784};
  // thread count. sp. thread_count =1 only use the main thread.
  int thread_count{1};
  // count of sample light
  int spp{16};
  // max_times to find the path
  int max_bounus{10};
  // the save name
  std::string save_name{"hw7_out.ppm"};
  // enable indirect light
  int enable_indirect{true};
  // epsilon for check
  float epsilon{0.001};

public:
  void ShowConfiguation() {
    std::cout << "=====================config====================" << std::endl;
    std::cout << "width: " << width << " height: " << height << std::endl;
    std::cout << "filename: " << save_name << std::endl;
    std::cout << "thread: " << thread_count << std::endl;
    std::cout << "spp: " << spp << std::endl;
    std::cout << "max_bounus: " << max_bounus << std::endl;
    std::cout << "enable_indirect_light: " << enable_indirect << std::endl;
    std::cout << "epsilon: " << epsilon << std::endl;
    std::cout << "==============================================" << std::endl;
  }

  static void ParseRuntimeArgs(int argc, char **argv, RuntimeArgs &args) {
    if (argc != 1) {
      for (int i = 0; i < argc; ++i) {
        if (strcmp(argv[i], "-h") == 0) {
          args.height = (int)atoi(argv[i + 1]);
        }
        if (strcmp(argv[i], "-w") == 0) {
          args.width = (int)atoi(argv[i + 1]);
        }
        if (strcmp(argv[i], "-spp") == 0) {
          args.spp = (int)atoi(argv[i + 1]);
        }
        if (strcmp(argv[i], "-mb") == 0) {
          args.max_bounus = (int)atoi(argv[i + 1]);
        }
        if (strcmp(argv[i], "-out_name") == 0) {
          args.save_name = argv[i + 1];
        }
        if (strcmp(argv[i], "-indirect") == 0) {
          args.enable_indirect = int(atoi(argv[i + 1]));
        }
        if (strcmp(argv[i], "-th") == 0) {
          args.thread_count = int(atoi(argv[i + 1]));
        }
        if (strcmp(argv[i], "-e") == 0) {
          args.thread_count = float(atof(argv[i + 1]));
        }
      }
    }
  }
};
extern RuntimeArgs global_args;
