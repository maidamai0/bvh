#pragma once

#include <algorithm>
#include <print>
#include <tuple>
#include <utility>
#include <vector>

#include "base.hpp"

struct split_point_uniform {
  static std::vector<float> candidates(const bvh_node& node, int axis,
                                       triangle_list&, std::vector<index_t>&) {
    int size = std::min(index_t(100), node.tri_count);
    float scale = node.bounds.extent(axis) / size;
    std::vector<float> candidates(size);
    std::generate_n(candidates.begin(), size,
                    [scale, cnt = 0, base = node.bounds.min[axis]]() mutable {
                      return base + scale * cnt++;
                    });
    return candidates;
  }
};

struct split_point_centroid {
  static std::vector<float> candidates(const bvh_node& node, int axis,
                                       triangle_list& triangles,
                                       std::vector<index_t>& indices) {
    std::vector<float> candidates(node.tri_count);
    index_t end = node.first_tri_idx + node.tri_count;
    for (index_t i = node.first_tri_idx; i < end; ++i) {
      auto& tri = triangles[indices[i]];
      candidates[i - node.first_tri_idx] = tri.centroid[axis];
    }
    return candidates;
  }
};

struct sah {
  sah(triangle_list& tris, std::vector<bvh_node>& nodes,
      std::vector<index_t>& indices)
      : triangles(tris), nodes(nodes), indices(indices) {}

  void split(index_t node_idx) {
    auto& node = nodes[node_idx];
    update_bounds(node_idx);

    // compute split axis and position
    const auto [split_axis, split_pos, best_cost] = split_point(node_idx);

    if (best_cost >= node.cost()) return;

    // split triangles into two halves
    index_t left = node.first_tri_idx;
    index_t right = left + node.tri_count - 1;
    while (left <= right) {
      if (triangles[indices[left]].centroid[split_axis] < split_pos) {
        ++left;
      } else {
        std::swap(indices[left], indices[right--]);
      }
    }

    // create child nodes for each half
    index_t left_count = left - node.first_tri_idx;
    if (left_count == 0 || left_count == node.tri_count) return;

    index_t left_node_idx = not_used++;
    index_t right_node_idx = not_used++;
    nodes[left_node_idx].first_tri_idx = node.first_tri_idx;
    nodes[left_node_idx].tri_count = left_count;
    nodes[right_node_idx].first_tri_idx = left;
    nodes[right_node_idx].tri_count = node.tri_count - left_count;

    node.left_node = left_node_idx;
    node.tri_count = 0;

    split(left_node_idx);
    split(right_node_idx);
  }

  void update_bounds(index_t node_idx) {
    bvh_node& node = nodes[node_idx];
    index_t end = node.first_tri_idx + node.tri_count;
    for (index_t i = node.first_tri_idx; i < end; ++i) {
      auto& tri = triangles[indices[i]];
      node.bounds.grow(tri.vertex0);
      node.bounds.grow(tri.vertex1);
      node.bounds.grow(tri.vertex2);
    }
  }

  std::tuple<int, float, float> split_point(index_t node_idx) {
    const bvh_node& node = nodes[node_idx];
    int best_axis = -1;
    float best_pos = 0.0f;
    float best_cost = max_v<float>;
    for (int axis = 0; axis < 3; ++axis) {
      const auto candidates =
          split_point_centroid::candidates(node, axis, triangles, indices);
      for (const auto pos : candidates) {
        float cost = sah_cost(node, axis, pos);
        if (cost < best_cost) {
          best_axis = axis;
          best_pos = pos;
          best_cost = cost;
        }
      }
    }

    return std::make_tuple(best_axis, best_pos, best_cost);
  }

  float sah_cost(const bvh_node& node, int axis, float pos) {
    aabb left_box, right_box;
    int left_cnt = 0, right_cnt = 0;
    index_t i = node.first_tri_idx;
    index_t end = node.first_tri_idx + node.tri_count;
    for (; i < end; ++i) {
      const auto& tri = triangles[indices[i]];
      if (tri.centroid[axis] < pos) {
        ++left_cnt;
        left_box.grow(tri.vertex0);
        left_box.grow(tri.vertex1);
        left_box.grow(tri.vertex2);
      } else {
        ++right_cnt;
        right_box.grow(tri.vertex0);
        right_box.grow(tri.vertex1);
        right_box.grow(tri.vertex2);
      }
    }
    float cost = left_cnt * left_box.area() + right_cnt * right_box.area();
    return cost > 0 ? cost : 1e30f;
  }

  triangle_list& triangles;
  std::vector<bvh_node>& nodes;
  std::vector<index_t>& indices;

  index_t not_used = 2;
};