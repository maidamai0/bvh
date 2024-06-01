#pragma once

#include <numeric>
#include <vector>

#include "base.hpp"

template <typename T>
concept bvh_strategy =
    requires(T t, index_t i, triangle_list& tris, std::vector<bvh_node>& nodes,
             std::vector<index_t>& indices) {
      { t.split(i) } -> std::same_as<void>;
      T{tris, nodes, indices};
    };

template <bvh_strategy Strategy>
struct bvh {
  bvh(triangle_list& tris) : triangles(tris) { build(); }

  void intersect(ray& r) const;

 private:
  void build();

  triangle_list& triangles;
  std::vector<bvh_node> nodes;
  std::vector<index_t> indices;
};

template <bvh_strategy Strategy>
void bvh<Strategy>::intersect(ray& r) const {
  const bvh_node* node = &nodes[0];
  std::array<const bvh_node*, 64> stack{};
  index_t stack_idx = 0;

  while (true) {
    if (node->is_leaf()) {
      for (int i = node->first_tri_idx;
           i < (node->first_tri_idx + node->tri_count); ++i) {
        intersect_tri(triangles[indices[i]], r);
      }
      if (stack_idx == 0) break;
      node = stack[--stack_idx];
      continue;
    }

    const bvh_node* child1 = &nodes[node->left_node];
    const bvh_node* child2 = &nodes[node->left_node + 1];
    float dist1 = child1->bounds.intersect2(r);
    float dist2 = child2->bounds.intersect2(r);
    if (dist1 > dist2) {
      std::swap(dist1, dist2);
      std::swap(child1, child2);
    }

    if (dist1 == 1e30f) {
      if (stack_idx == 0) break;
      node = stack[--stack_idx];
      continue;
    }

    node = child1;
    if (dist2 < 1e30f) stack[stack_idx++] = child2;
  }
}

template <bvh_strategy Strategy>
void bvh<Strategy>::build() {
  TRACE;

  indices.resize(triangles.size());
  std::iota(indices.begin(), indices.end(), 0);

  // compute centroids
  for (auto& tri : triangles) {
    tri.centroid = (tri.vertex0 + tri.vertex1 + tri.vertex2) * 0.3333f;
  }

  nodes.resize(triangles.size() * 2);
  bvh_node& root = nodes[0];
  root.first_tri_idx = 0;
  root.tri_count = triangles.size();

  Strategy strategy(triangles, nodes, indices);
  strategy.split(0);
}