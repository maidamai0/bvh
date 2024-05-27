#include "bvh.hpp"

#include <cstdio>
#include <cstdlib>

#include "viewer.h"

int main(int argc, char** argv) {
  auto triangles = make_triangles(64);
  basic_bvh bvh(triangles);
  run("basic bvh", 1024, 512, [&](Surface& canvas) {
    timer timer;
    float3 cam_pos{0, 0, -18};
    float3 p0{-1, 1, -15}, p1{1, 1, -15}, p2{-1, -1, -15};
    for (int y = 0; y < canvas.height; ++y) {
      for (int x = 0; x < canvas.width; ++x) {
        float u = x / float(canvas.width);
        float v = y / float(canvas.height);
        float3 pixel_pos = p0 + (p1 - p0) * u + (p2 - p0) * v;
        ray r = ray{cam_pos, normalize(pixel_pos - cam_pos)};
        // bvh.intersect(r);
        for (auto& t : triangles) intersect_tri(t, r);
        if (r.t < 1e30f) canvas.Plot(x, y, 0x0000ff);
      }
    }

    std::println("tracing time: {}ms ({}K rays/s)", timer.elapsed(),
                 float(canvas.width * canvas.height) / timer.elapsed());
  });

  return EXIT_SUCCESS;
}