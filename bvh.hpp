#pragma once

#include <numeric>
#include <vector>

#include "base.hpp"

struct basic_bvh {
  basic_bvh(triangle_list& tris) : triangles(tris) { build(); }
  void intersect(ray& r) const { intersect_impl(r, 0); }

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

    nodes.resize(triangles.size() * 2 - 1);
    bvh_node& root = nodes[0];
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
    if (node.tri_count <= 2) return;

    // compute split axis and position
    float3 extent = node.bounds.max - node.bounds.min;
    int split_axis = 0;
    if (extent.y > extent[split_axis]) split_axis = 1;
    if (extent.z > extent[split_axis]) split_axis = 2;
    float split_pos = node.bounds.center(split_axis);

    // split triangles into two halves
    index_t left = node.first_tri_idx;
    index_t right = left + node.tri_count;
    while (left < right) {
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
    node.left_node = left_node_idx;
    nodes[left_node_idx].first_tri_idx = node.first_tri_idx;
    nodes[left_node_idx].tri_count = left_count;
    nodes[right_node_idx].first_tri_idx = left;
    nodes[right_node_idx].tri_count = node.tri_count - left_count;

    update_bounds(left_node_idx);
    update_bounds(right_node_idx);
    split(left_node_idx);
    split(right_node_idx);
  }

  std::vector<bvh_node> nodes;
  std::vector<index_t> indices;
  index_t not_used = 0;

  triangle_list& triangles;
};