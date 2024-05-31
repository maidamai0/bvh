#pragma once

#include <array>
#include <numeric>
#include <print>
#include <tuple>
#include <utility>
#include <vector>

#include "base.hpp"

struct sah_bvh {
  sah_bvh(triangle_list& tris) : triangles(tris) { build(); }
  void intersect(ray& r) const { intersect_impl(r, 0); }

  // broken
  void intersect2(ray& r) const {
    const bvh_node* node = &nodes[0];
    std::array<const bvh_node*, 64> stack{};
    index_t stack_idx = 0;

    while (true) {
      if (node->is_leaf()) {
        for (index_t i = 0; i < node->tri_count; ++i) {
          intersect_tri(triangles[node->first_tri_idx + i], r);
        }

        if (stack_idx == 0)
          break;
        else
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
        if (stack_idx == 0)
          break;
        else
          node = stack[--stack_idx];
      } else {
        node = child1;
        if (dist2 != 1e30f) stack[stack_idx++] = child2;
      }
    }
  }

 private:
  void intersect_impl(ray& r, index_t node_idx) const {
    const bvh_node& node = nodes[node_idx];
    if (!node.bounds.intersect(r)) return;

    if (node.is_leaf()) {
      for (index_t i = 0; i < node.tri_count; ++i) {
        intersect_tri(triangles[indices[node.first_tri_idx + i]], r);
      }
      return;
    }

    intersect_impl(r, node.left_node);
    intersect_impl(r, node.left_node + 1);
  }

  void build() {
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
    update_bounds(0);
    split(0);
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

  void split(index_t node_idx) {
    auto& node = nodes[node_idx];

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

    update_bounds(left_node_idx);
    update_bounds(right_node_idx);
    split(left_node_idx);
    split(right_node_idx);
  }

  std::tuple<int, float, float> split_point(index_t node_idx) {
    const bvh_node& node = nodes[node_idx];
    int best_axis = -1;
    float best_pos = 0.0f;
    float best_cost = 1e30f;
    auto end = node.tri_count + node.first_tri_idx;
    for (int axis = 0; axis < 3; ++axis) {
      for (index_t i = node.first_tri_idx; i < end; ++i) {
        auto& tri = triangles[indices[i]];
        float pos = tri.centroid[axis];
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

  std::vector<bvh_node> nodes;
  std::vector<index_t> indices;
  index_t not_used = 2;

  triangle_list& triangles;
};