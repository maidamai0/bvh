#include <print>

#include "base.hpp"
#include "bvh.hpp"

int main(int argc, char** argv) {
  auto triangles = make_triangles(1024);
  basic_bvh bvh(triangles);
  ray r;
  {
    timer trace("basic bvh");
    float3 cam_pos{0, 0, -18};
    float3 p0{-1, 1, -15}, p1{1, 1, -15}, p2{-1, -1, -15};
    int width = 640, height = 640;
    for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {
        float u = x / float(width);
        float v = y / float(height);
        float3 pixel_pos = p0 + (p1 - p0) * u + (p2 - p0) * v;
        r = ray{cam_pos, normalize(pixel_pos - cam_pos)};

        for (const auto& triangle : triangles) intersect_tri(triangle, r);
        // bvh.intersect(r);
      }
    }
  }

  std::println("t: {}", r.t);
  return 0;
}