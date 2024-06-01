#pragma once

#include <vector>

#include "base.hpp"

struct middle_point {
  middle_point(triangle_list& tris, std::vector<bvh_node>& nodes,
               std::vector<index_t>& indices)
      : triangles(tris), nodes(nodes), indices(indices) {}

  void split(index_t node_idx) {
    auto& node = nodes[node_idx];
    if (node.tri_count <= 2) return;

    update_bounds(node_idx);

    // compute split axis and position
    float3 extent = node.bounds.max - node.bounds.min;
    int split_axis = 0;
    if (extent.y > extent[split_axis]) split_axis = 1;
    if (extent.z > extent[split_axis]) split_axis = 2;
    float split_pos = node.bounds.center(split_axis);

    // split triangles into two halves
    index_t left = node.first_tri_idx;
    index_t right = left + node.tri_count - 1;
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

    split(left_node_idx);
    split(right_node_idx);
  }

 private:
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
  triangle_list& triangles;

  std::vector<bvh_node> nodes;
  std::vector<index_t> indices;
  index_t not_used = 0;
};