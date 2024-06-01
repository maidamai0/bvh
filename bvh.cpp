#include "bvh.hpp"

#include <cstdlib>
#include <execution>
#include <numeric>

#include "basic.hpp"
#include "model.hpp"
#include "sah.hpp"
#include "viewer.h"

int show_random_triangles() {
  auto triangles = make_triangles(64);
  bvh<middle_point> bvh(triangles);
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

  std::exit(EXIT_SUCCESS);
}
int show_unity() {
  auto triangles = unity_model();
  bvh<sah> bvh(triangles);
  run("sah bvh", 640, 640, [&](Surface& canvas) {
    timer timer;

    canvas.Clear(0);
    float3 cam_pos(-1.5f, -0.2f, -2.5f);
    float3 p0(-2.5f, 0.8f, -0.5f);
    float3 p1(-0.5f, 0.8f, -0.5f);
    float3 p2(-2.5f, -1.2f, -0.5f);

    int len = canvas.height * canvas.width;
    std::vector<int> indices(len);
    std::iota(indices.begin(), indices.end(), 0);
    std::for_each_n(std::execution::par, indices.begin(), len, [&](int& i) {
      int x = i % canvas.width;
      int y = i / canvas.width;
      float u = x / float(canvas.width);
      float v = y / float(canvas.height);
      float3 pixel_pos = p0 + (p1 - p0) * u + (p2 - p0) * v;
      ray r = ray{cam_pos, normalize(pixel_pos - cam_pos)};
      bvh.intersect(r);
      uint32_t c = 500 - (int)(r.t * 20);
      if (r.t < 1e30f) canvas.Plot(x, y, c * 0x10101);
    });

    std::println("tracing time: {}ms ({}M rays/s)", timer.elapsed(),
                 float(canvas.width * canvas.height) / timer.elapsed() / 1000);
  });

  std::exit(EXIT_SUCCESS);
}

int main(int argc, char** argv) {
  int i = 0;
  if (argc > 1) {
    i = atoi(argv[1]);
  }

  if (i == 0) show_random_triangles();
  if (i == 1) show_unity();

  show_unity();

  return 0;
}