#include "sah.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <execution>
#include <fstream>
#include <numeric>
#include <print>
#include <vector>

#include "viewer.h"

triangle_list load_model() {
  std::ifstream ifs("unity.tri");
  if (!ifs.is_open()) {
    std::println("failed to open unity.tri");
    throw std::runtime_error("failed to open unity.tri");
  }

  triangle_list triangles;
  while (true) {
    std::array<float, 9> points{};
    for (auto& p : points) {
      ifs >> p;
    }
    if (points[0] == 999) {
      break;
    }
    float3 v0{points[0], points[1], points[2]};
    float3 v1{points[3], points[4], points[5]};
    float3 v2{points[6], points[7], points[8]};
    triangles.push_back({v0, v1, v2});
  }

  std::println("loaded {} triangles", triangles.size());
  return triangles;
}

int main(int argc, char** argv) {
  auto triangles = load_model();
  sah_bvh bvh(triangles);
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

    std::println("tracing time: {}ms ({}K rays/s)", timer.elapsed(),
                 float(canvas.width * canvas.height) / timer.elapsed());
  });

  return EXIT_SUCCESS;
}