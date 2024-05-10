#include "BVH.hpp"
#include "Bounds3.hpp"
#include "Intersection.hpp"
#include "Vector.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>

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
    Bounds3 centroidBounds;
    for (int i = 0; i < objects.size(); ++i)
      centroidBounds =
          Union(centroidBounds, objects[i]->getBounds().Centroid());
    if (splitMethod == SplitMethod::NAIVE) {
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

struct SAHBucket {
  float end_split{0};
  Bounds3 bounds;
  int prim_count{0};
};

void BVHAccel::BuildSAH(std::vector<Object *> objects, BVHBuildNode *node) {
  return;
  Bounds3 bounds;
  for (int i = 0; i < objects.size(); ++i)
    bounds = Union(bounds, objects[i]->getBounds());

  const int BacketCount = 8;
  std::array<SAHBucket, BacketCount> backets;

  auto compute_bucket = [](const Vector3f &centroid, int index,
                           std::array<SAHBucket, BacketCount> &backets) {
    for (int i = 0; i < BacketCount; ++i) {
      if (backets[i].end_split > centroid[index]) {
        return i;
      }
    }
    return BacketCount - 1;
  };

  for (int i = 0; i < 3; i++) {
    // init backet
    for (int j = 0; j < BacketCount; ++j) {
      backets[j] = SAHBucket();
      backets[j].end_split = bounds.pMin.x + (bounds.pMax.x - bounds.pMin.x) *
                                                 (j + 1.f) / BacketCount;
    }

    for (auto obj : objects) {
      int b = compute_bucket(obj->getBounds().Centroid(), i, backets);
      backets[b].bounds = Union(backets[b].bounds, obj->getBounds());
      backets[b].prim_count++;
    }
  }
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

  // recursive part
  auto left_inter = getIntersection(node->left, ray);
  auto right_inter = getIntersection(node->right, ray);
  return left_inter.distance < right_inter.distance ? left_inter : right_inter;
}