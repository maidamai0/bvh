#pragma once

#include <fstream>

#include "base.hpp"

inline triangle_list make_triangles(size_t size = 64) {
  triangle_list triangles;
  triangles.resize(size);
  for (auto& tri : triangles) {
    float3 r0(random_float(), random_float(), random_float());
    float3 r1(random_float(), random_float(), random_float());
    float3 r2(random_float(), random_float(), random_float());

    tri.vertex0 = r0 * 9 - float3(5);
    tri.vertex1 = tri.vertex0 + r1;
    tri.vertex2 = tri.vertex0 + r2;
  }

  return triangles;
}

inline triangle_list unity_model() {
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
