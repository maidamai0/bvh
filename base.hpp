#pragma once

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <limits>
#include <print>
#include <vector>

template <typename T>
constexpr T infinity_v = std::numeric_limits<T>::infinity();
template <typename T>
constexpr T max_v = std::numeric_limits<T>::max();
template <typename T>
constexpr T min_v = std::numeric_limits<T>::min();

using index_t = uint32_t;

struct timer {
  using clock_t = std::chrono::steady_clock;
  using time_point_t = clock_t::time_point;

  timer() { start = clock_t::now(); }

  float elapsed() {
    using namespace std::chrono;
    auto elapsed_ms = duration_cast<microseconds>(clock_t::now() - start);
    return static_cast<float>(elapsed_ms.count()) / 1000;
  }

  timer &operator=(const timer &) = delete;
  timer(const timer &) = delete;

  time_point_t start;
};

struct scope_timer : timer {
  scope_timer(const char *name) : name(name) {}
  ~scope_timer() { std::println("{} cost: {}ms", name, elapsed()); }
  const char *name;
};

#define TRACE scope_timer __scope_timer__(__FUNCTION__);

uint32_t inline random_uint(uint32_t &seed) {
  seed ^= (seed << 13);
  seed ^= (seed >> 17);
  seed ^= (seed << 5);
  return seed;
}

uint32_t inline random_uint() {
  static uint32_t seed = 0x12345678;
  return random_uint(seed);
}

float inline random_float() {
  return static_cast<float>(random_uint()) * 2.3283064365387e-10f;
}
float inline random_float(uint32_t &seed) {
  return static_cast<float>(random_uint(seed)) * 2.3283064365387e-10f;
}

// float3
struct float3 {
  float3() = default;
  float3(float x, float y, float z) : x(x), y(y), z(z) {}
  float3(float v) : x(v), y(v), z(v) {}

  float &operator[](int i) { return cell[i]; }
  float operator[](int i) const { return cell[i]; }

  union {
    struct {
      float x, y, z;
    };
    float cell[3];
  };
};

inline float3 operator+(const float3 &a, const float3 &b) {
  return float3(a.x + b.x, a.y + b.y, a.z + b.z);
}

inline float3 operator-(const float3 &a) { return float3(-a.x, -a.y, -a.z); }
inline float3 operator-(const float3 &a, const float3 &b) {
  return float3(a.x - b.x, a.y - b.y, a.z - b.z);
}

inline float3 operator*(const float3 &a, const float3 &b) {
  return float3(a.x * b.x, a.y * b.y, a.z * b.z);
}
inline void operator*=(float3 &a, const float3 &b) {
  a.x *= b.x;
  a.y *= b.y;
  a.z *= b.z;
}
inline float3 operator*(const float3 &a, float b) {
  return float3(a.x * b, a.y * b, a.z * b);
}
inline float3 operator*(float b, const float3 &a) {
  return float3(b * a.x, b * a.y, b * a.z);
}
inline void operator*=(float3 &a, float b) {
  a.x *= b;
  a.y *= b;
  a.z *= b;
}

inline float3 operator/(const float3 &a, float b) {
  return float3(a.x / b, a.y / b, a.z / b);
}

inline float3 cross(const float3 &a, const float3 &b) {
  return float3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z,
                a.x * b.y - a.y * b.x);
}

inline float dot(const float3 &a, const float3 &b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline float3 normalize(const float3 &a) {
  float invert_len = 1 / std::sqrt(dot(a, a));
  return a * invert_len;
}

inline float3 min(const float3 &a, const float3 &b) {
  return float3(std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z));
}

inline float3 max(const float3 &a, const float3 &b) {
  return float3(std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z));
}

// end float3

struct triangle {
  float3 vertex0, vertex1, vertex2, centroid;
};
using triangle_list = std::vector<triangle>;

struct ray {
  float3 origin, direction;
  float t = infinity_v<float>;
};

struct aabb {
  void grow(const float3 &p) {
    min = ::min(min, p);
    max = ::max(max, p);
  }

  void grow(const aabb &b) {
    grow(b.min);
    grow(b.max);
  }

  float area() const {
    float3 d = max - min;
    return d.x * d.y + d.x * d.z + d.y * d.z;
  }

  bool intersect(const ray &ray) const {
    float tx1 = (min.x - ray.origin.x) / ray.direction.x;
    float tx2 = (max.x - ray.origin.x) / ray.direction.x;
    float tmin = std::min(tx1, tx2), tmax = std::max(tx1, tx2);

    float ty1 = (min.y - ray.origin.y) / ray.direction.y;
    float ty2 = (max.y - ray.origin.y) / ray.direction.y;
    tmin = std::max(tmin, std::min(ty1, ty2));
    tmax = std::min(tmax, std::max(ty1, ty2));

    float tz1 = (min.z - ray.origin.z) / ray.direction.z;
    float tz2 = (max.z - ray.origin.z) / ray.direction.z;
    tmin = std::max(tmin, std::min(tz1, tz2));
    tmax = std::min(tmax, std::max(tz1, tz2));

    return tmax >= tmin && tmin < ray.t && tmax > 0;
  }

  float center(int axis) const { return (min[axis] + max[axis]) * 0.5f; }
  float3 center() const { return (min + max) * 0.5f; }

  float3 min{max_v<float>}, max{min_v<float>};
};

struct bvh_node {
  aabb bounds;              // 24 bytes
  index_t left_node{};      // 4 bytes, right child is always left_child + 1
  index_t first_tri_idx{};  // 4 bytes
  index_t tri_count{};      // 4 bytes

  bool is_leaf() const { return tri_count > 0; }
};

void inline intersect_tri(const triangle &t, ray &r) {
  float3 e1 = t.vertex1 - t.vertex0;
  float3 e2 = t.vertex2 - t.vertex0;
  float3 p = cross(r.direction, e2);

  float det = dot(e1, p);
  if (det - 1e-4f && det < -1e-4f) return;

  float inv_det = 1 / det;
  float3 tvec = r.origin - t.vertex0;

  float u = dot(tvec, p) * inv_det;
  if (u < 0 || u > 1) return;

  float3 q = cross(tvec, e1);
  float v = dot(r.direction, q) * inv_det;
  if (v < 0 || u + v > 1) return;

  float tt = dot(e2, q) * inv_det;
  if (tt > 1e-4f) r.t = std::min(r.t, tt);
  // std::println("t: {}", r.t);
}

inline triangle_list make_triangles(size_t size = 64) {
  triangle_list triangles;
  triangles.resize(size);
  for (auto &tri : triangles) {
    float3 r0(random_float(), random_float(), random_float());
    float3 r1(random_float(), random_float(), random_float());
    float3 r2(random_float(), random_float(), random_float());

    tri.vertex0 = r0 * 9 - float3(5);
    tri.vertex1 = tri.vertex0 + r1;
    tri.vertex2 = tri.vertex0 + r2;
  }

  return triangles;
}