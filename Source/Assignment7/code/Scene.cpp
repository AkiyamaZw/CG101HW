//
// Created by Göksu Güvendiren on 2019-05-14.
//

#include "Scene.hpp"
#include "Intersection.hpp"
#include "Vector.hpp"
#include "global.hpp"
#include <cmath>

void Scene::buildBVH() {
  printf(" - Generating BVH...\n\n");
  this->bvh = new BVHAccel(objects, 1, BVHAccel::SplitMethod::NAIVE);
}

Intersection Scene::intersect(const Ray &ray) const {
  return this->bvh->Intersect(ray);
}

void Scene::sampleLight(Intersection &pos, float &pdf) const {
  float emit_area_sum = 0;
  for (uint32_t k = 0; k < objects.size(); ++k) {
    if (objects[k]->hasEmit()) {
      emit_area_sum += objects[k]->getArea();
    }
  }
  float p = get_random_float() * emit_area_sum;
  emit_area_sum = 0;
  for (uint32_t k = 0; k < objects.size(); ++k) {
    if (objects[k]->hasEmit()) {
      emit_area_sum += objects[k]->getArea();
      if (p <= emit_area_sum) {
        objects[k]->Sample(pos, pdf);
        break;
      }
    }
  }
}

bool Scene::trace(const Ray &ray, const std::vector<Object *> &objects,
                  float &tNear, uint32_t &index, Object **hitObject) {
  *hitObject = nullptr;
  for (uint32_t k = 0; k < objects.size(); ++k) {
    float tNearK = kInfinity;
    uint32_t indexK;
    Vector2f uvK;
    if (objects[k]->intersect(ray, tNearK, indexK) && tNearK < tNear) {
      *hitObject = objects[k];
      tNear = tNearK;
      index = indexK;
    }
  }

  return (*hitObject != nullptr);
}

// Implementation of Path Tracing
Vector3f Scene::castRay(const Ray &ray, int depth) const {
  // TO DO Implement Path Tracing Algorithm here
  // pre check, the exit of recursion
  if (depth >= global_args.max_bounus) {
    std::cout << "reach the max bounus" << std::endl;
    return {};
  }

  Vector3f direct_color{0.f}, indirect_color{0.f};

  // reflection plane: r is reflection for short.
  Intersection r_inter = intersect(ray);
  if (!r_inter.happened)
    return {};

  // self-emmition part
  if (r_inter.m->hasEmission())
    return r_inter.m->getEmission();

  // light sample: l is light for short.
  Intersection l_inter;
  float l_pdf = 0.f;
  sampleLight(l_inter, l_pdf);
  float ws_dis = (l_inter.coords - r_inter.coords).norm();
  Vector3f l_dir = (l_inter.coords - r_inter.coords)
                       .normalized(); // from reflect plane to light plane

  // start path tracer alg.
  // 1. direct light check light object intersection
  Ray l_to_r(l_inter.coords, -l_dir);
  Intersection l_to_r_inter = intersect(l_to_r);

  // there are no objects between light point to reflect point.
  if (l_to_r_inter.distance - ws_dis > -global_args.epsilon) {
    direct_color =
        l_inter.emit * r_inter.m->eval(ray.direction, l_dir, r_inter.normal) *
        dotProduct(r_inter.normal, l_dir) * dotProduct(l_inter.normal, -l_dir) /
        (ws_dis * ws_dis) / (l_pdf);
  }

  // 2. indirect light
  if (global_args.enable_indirect) {
    float russian_prob = get_random_float();
    if (russian_prob < RussianRoulette) {
      Vector3f wi =
          r_inter.m->sample(ray.direction, r_inter.normal).normalized();
      Ray r(r_inter.coords, wi);
      auto sample_inter = intersect(r);
      if (sample_inter.happened && !sample_inter.m->hasEmission()) {
        indirect_color = castRay(r, depth + 1) *
                         r_inter.m->eval(ray.direction, wi, r_inter.normal) *
                         dotProduct(wi, r_inter.normal) /
                         r_inter.m->pdf(r.direction, wi, r_inter.normal) /
                         RussianRoulette;
      }
    }
  }

  return direct_color + indirect_color;
}