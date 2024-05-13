#include "BVH.hpp"
#include "Bounds3.hpp"
#include "Intersection.hpp"
#include "Vector.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <limits>
#include <vector>

BVHAccel::BVHAccel(std::vector<Object *> p, int maxPrimsInNode,
                   SplitMethod splitMethod)
    : maxPrimsInNode(std::min(255, maxPrimsInNode)), splitMethod(splitMethod),
      primitives(std::move(p)) {
  time_t start, stop;
  time(&start);
  if (primitives.empty())
    return;

  root = recursiveBuild(primitives);

  time(&stop);
  double diff = difftime(stop, start);
  int hrs = (int)diff / 3600;
  int mins = ((int)diff / 60) - (hrs * 60);
  int secs = (int)diff - (hrs * 3600) - (mins * 60);

  printf(
      "\rBVH Generation complete: \nTime Taken: %i hrs, %i mins, %i secs\n\n",
      hrs, mins, secs);
}

BVHBuildNode *BVHAccel::recursiveBuild(std::vector<Object *> objects) {
  BVHBuildNode *node = new BVHBuildNode();

  // Compute bounds of all primitives in BVH node
  Bounds3 bounds;
  for (int i = 0; i < objects.size(); ++i)
    bounds = Union(bounds, objects[i]->getBounds());
  if (objects.size() == 1) {
    // Create leaf _BVHBuildNode_
    node->bounds = objects[0]->getBounds();
    node->object = objects[0];
    node->left = nullptr;
    node->right = nullptr;
    return node;
  } else if (objects.size() == 2) {
    node->left = recursiveBuild(std::vector{objects[0]});
    node->right = recursiveBuild(std::vector{objects[1]});

    node->bounds = Union(node->left->bounds, node->right->bounds);
    return node;
  } else {
    if (splitMethod == SplitMethod::NAIVE) {
      Bounds3 centroidBounds;
      for (int i = 0; i < objects.size(); ++i)
        centroidBounds =
            Union(centroidBounds, objects[i]->getBounds().Centroid());
      int dim = centroidBounds.maxExtent();
      switch (dim) {
      case 0:
        std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
          return f1->getBounds().Centroid().x < f2->getBounds().Centroid().x;
        });
        break;
      case 1:
        std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
          return f1->getBounds().Centroid().y < f2->getBounds().Centroid().y;
        });
        break;
      case 2:
        std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
          return f1->getBounds().Centroid().z < f2->getBounds().Centroid().z;
        });
        break;
      }

      auto beginning = objects.begin();
      auto middling = objects.begin() + (objects.size() / 2);
      auto ending = objects.end();

      auto leftshapes = std::vector<Object *>(beginning, middling);
      auto rightshapes = std::vector<Object *>(middling, ending);

      assert(objects.size() == (leftshapes.size() + rightshapes.size()));

      node->left = recursiveBuild(leftshapes);
      node->right = recursiveBuild(rightshapes);

      node->bounds = Union(node->left->bounds, node->right->bounds);
    } else if (splitMethod == SplitMethod::SAH) {
      BuildSAH(objects, node);
    } else {
      assert(false);
    }
  }

  return node;
}

void BVHAccel::BuildSAH(std::vector<Object *> objects, BVHBuildNode *node) {
  const int BacketCount = 8;
  const float t_const = 0.125f;
  std::array<float, BacketCount - 1> splits;

  Bounds3 centroidBounds;
  for (int i = 0; i < objects.size(); ++i)
    centroidBounds = Union(centroidBounds, objects[i]->getBounds().Centroid());
  int dim = centroidBounds.maxExtent();

  for (int i = 0; i < BacketCount - 1; i++) {
    splits[i] = (i + 1) *
                    (centroidBounds.pMax[dim] - centroidBounds.pMin[dim]) /
                    (BacketCount - 1) +
                centroidBounds.pMin[dim];
  }

  // split along with the wider axis
  float min_cost = std::numeric_limits<float>::max();
  int min_backet = 0;
  std::vector<Object *>::iterator min_left_end_iter = objects.end();
  for (int i = 0; i < BacketCount - 1; i++) {
    float split_plane = splits[i];
    auto left_end = std::partition(
        objects.begin(), objects.end(), [dim, split_plane](auto f1) {
          return f1->getBounds().Centroid()[dim] < split_plane;
        });
    Bounds3 left_bounds, right_bounds;
    for (auto it = objects.begin(); it != left_end; it++) {
      left_bounds = Union(left_bounds, (*it)->getBounds().Centroid());
    }
    for (auto it = left_end; it != objects.end(); it++) {
      right_bounds = Union(right_bounds, (*it)->getBounds().Centroid());
    }
    float cost =
        t_const + (left_bounds.SurfaceArea() * (left_end - objects.begin()) +
                   right_bounds.SurfaceArea() * (objects.end() - left_end)) /
                      centroidBounds.SurfaceArea();
    if (cost < min_cost) {
      min_cost = cost;
      min_left_end_iter = left_end;
    }
  }

  auto left_shapes = std::vector<Object *>(objects.begin(), min_left_end_iter);
  auto right_shapes = std::vector<Object *>(min_left_end_iter, objects.end());
  assert(objects.size() == left_shapes.size() + right_shapes.size());
  node->left = recursiveBuild(left_shapes);
  node->right = recursiveBuild(right_shapes);
  node->bounds = Union(node->left->bounds, node->right->bounds);
}

Intersection BVHAccel::Intersect(const Ray &ray) const {
  Intersection isect;
  if (!root)
    return isect;
  isect = BVHAccel::getIntersection(root, ray);
  return isect;
}

Intersection BVHAccel::getIntersection(BVHBuildNode *node,
                                       const Ray &ray) const {
  // TODO Traverse the BVH to find intersection
  // recursive exit
  if (node == nullptr || !node->bounds.IntersectP(ray, ray.direction_inv,
                                                  {int(ray.direction.x < 0),
                                                   int(ray.direction.y < 0),
                                                   int(ray.direction.z < 0)}))
    return {};

  // leave node ray&object intersection check
  if (node->left == nullptr && node->right == nullptr) {
    return node->object->getIntersection(ray);
  }

  // recursive
  auto left_inter = getIntersection(node->left, ray);
  auto right_inter = getIntersection(node->right, ray);
  return left_inter.distance < right_inter.distance ? left_inter : right_inter;
}