#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <execution>
#include <numeric>
#include <print>
#include <vector>

#include "viewer.h"
#include "xmmintrin.h"

// math
// ==============================================================================
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>

using uint = uint32_t;
using uchar = unsigned char;

// vector type placeholders, carefully matching OpenCL's layout and alignment
struct alignas(8) int2 {
  int2() = default;
  int2(const int a, const int b) : x(a), y(b) {}
  int2(const int a) : x(a), y(a) {}
  union {
    struct {
      int x, y;
    };
    int cell[2];
  };
  int& operator[](const int n) { return cell[n]; }
};
struct alignas(8) uint2 {
  uint2() = default;
  uint2(const int a, const int b) : x(a), y(b) {}
  uint2(const uint a) : x(a), y(a) {}
  union {
    struct {
      uint x, y;
    };
    uint cell[2];
  };
  uint& operator[](const int n) { return cell[n]; }
};
struct alignas(8) float2 {
  float2() = default;
  float2(const float a, const float b) : x(a), y(b) {}
  float2(const float a) : x(a), y(a) {}
  union {
    struct {
      float x, y;
    };
    float cell[2];
  };
  float& operator[](const int n) { return cell[n]; }
};
struct int3;
struct alignas(16) int4 {
  int4() = default;
  int4(const int a, const int b, const int c, const int d)
      : x(a), y(b), z(c), w(d) {}
  int4(const int a) : x(a), y(a), z(a), w(a) {}
  int4(const int3& a, const int d);
  union {
    struct {
      int x, y, z, w;
    };
    int cell[4];
  };
  int& operator[](const int n) { return cell[n]; }
};
struct alignas(16) int3 {
  int3() = default;
  int3(const int a, const int b, const int c) : x(a), y(b), z(c) {}
  int3(const int a) : x(a), y(a), z(a) {}
  int3(const int4 a) : x(a.x), y(a.y), z(a.z) {}
  union {
    struct {
      int x, y, z;
      int dummy;
    };
    int cell[4];
  };
  int& operator[](const int n) { return cell[n]; }
};
struct uint3;
struct alignas(16) uint4 {
  uint4() = default;
  uint4(const uint a, const uint b, const uint c, const uint d)
      : x(a), y(b), z(c), w(d) {}
  uint4(const uint a) : x(a), y(a), z(a), w(a) {}
  uint4(const uint3& a, const uint d);
  union {
    struct {
      uint x, y, z, w;
    };
    uint cell[4];
  };
  uint& operator[](const int n) { return cell[n]; }
};
struct alignas(16) uint3 {
  uint3() = default;
  uint3(const uint a, const uint b, const uint c) : x(a), y(b), z(c) {}
  uint3(const uint a) : x(a), y(a), z(a) {}
  uint3(const uint4 a) : x(a.x), y(a.y), z(a.z) {}
  union {
    struct {
      uint x, y, z;
      uint dummy;
    };
    uint cell[4];
  };
  uint& operator[](const int n) { return cell[n]; }
};
struct float3;
struct alignas(16) float4 {
  float4() = default;
  float4(const float a, const float b, const float c, const float d)
      : x(a), y(b), z(c), w(d) {}
  float4(const float a) : x(a), y(a), z(a), w(a) {}
  float4(const float3& a, const float d);
  float4(const float3& a);
  union {
    struct {
      float x, y, z, w;
    };
    float cell[4];
  };
  float& operator[](const int n) { return cell[n]; }
};
struct float3 {
  float3() = default;
  float3(const float a, const float b, const float c) : x(a), y(b), z(c) {}
  float3(const float a) : x(a), y(a), z(a) {}
  float3(const float4 a) : x(a.x), y(a.y), z(a.z) {}
  float3(const uint3 a) : x((float)a.x), y((float)a.y), z((float)a.z) {}
  union {
    struct {
      float x, y, z;
    };
    float cell[3];
  };
  float& operator[](const int n) { return cell[n]; }
};
struct alignas(4) uchar4 {
  uchar4() = default;
  uchar4(const uchar a, const uchar b, const uchar c, const uchar d)
      : x(a), y(b), z(c), w(d) {}
  uchar4(const uchar a) : x(a), y(a), z(a), w(a) {}
  union {
    struct {
      uchar x, y, z, w;
    };
    uchar cell[4];
  };
  uchar& operator[](const int n) { return cell[n]; }
};

// math
inline float fminf(float a, float b) { return a < b ? a : b; }
inline float fmaxf(float a, float b) { return a > b ? a : b; }
inline float rsqrtf(float x) { return 1.0f / std::sqrtf(x); }
inline float sqrf(float x) { return x * x; }
inline int sqr(int x) { return x * x; }

inline float2 make_float2(const float a, float b) {
  float2 f2;
  f2.x = a, f2.y = b;
  return f2;
}
inline float2 make_float2(const float s) { return make_float2(s, s); }
inline float2 make_float2(const float3& a) { return make_float2(a.x, a.y); }
inline float2 make_float2(const int2& a) {
  return make_float2(float(a.x), float(a.y));
}  // explicit casts prevent gcc warnings
inline float2 make_float2(const uint2& a) {
  return make_float2(float(a.x), float(a.y));
}
inline int2 make_int2(const int a, const int b) {
  int2 i2;
  i2.x = a, i2.y = b;
  return i2;
}
inline int2 make_int2(const int s) { return make_int2(s, s); }
inline int2 make_int2(const int3& a) { return make_int2(a.x, a.y); }
inline int2 make_int2(const uint2& a) { return make_int2(int(a.x), int(a.y)); }
inline int2 make_int2(const float2& a) { return make_int2(int(a.x), int(a.y)); }
inline uint2 make_uint2(const uint a, const uint b) {
  uint2 u2;
  u2.x = a, u2.y = b;
  return u2;
}
inline uint2 make_uint2(const uint s) { return make_uint2(s, s); }
inline uint2 make_uint2(const uint3& a) { return make_uint2(a.x, a.y); }
inline uint2 make_uint2(const int2& a) {
  return make_uint2(uint(a.x), uint(a.y));
}
inline float3 make_float3(const float& a, const float& b, const float& c) {
  float3 f3;
  f3.x = a, f3.y = b, f3.z = c;
  return f3;
}
inline float3 make_float3(const float& s) { return make_float3(s, s, s); }
inline float3 make_float3(const float2& a) {
  return make_float3(a.x, a.y, 0.0f);
}
inline float3 make_float3(const float2& a, const float& s) {
  return make_float3(a.x, a.y, s);
}
inline float3 make_float3(const float4& a) {
  return make_float3(a.x, a.y, a.z);
}
inline float3 make_float3(const int3& a) {
  return make_float3(float(a.x), float(a.y), float(a.z));
}
inline float3 make_float3(const uint3& a) {
  return make_float3(float(a.x), float(a.y), float(a.z));
}
inline int3 make_int3(const int& a, const int& b, const int& c) {
  int3 i3;
  i3.x = a, i3.y = b, i3.z = c;
  return i3;
}
inline int3 make_int3(const int& s) { return make_int3(s, s, s); }
inline int3 make_int3(const int2& a) { return make_int3(a.x, a.y, 0); }
inline int3 make_int3(const int2& a, const int& s) {
  return make_int3(a.x, a.y, s);
}
inline int3 make_int3(const uint3& a) {
  return make_int3(int(a.x), int(a.y), int(a.z));
}
inline int3 make_int3(const float3& a) {
  return make_int3(int(a.x), int(a.y), int(a.z));
}
inline int3 make_int3(const float4& a) {
  return make_int3(int(a.x), int(a.y), int(a.z));
}
inline uint3 make_uint3(const uint a, uint b, uint c) {
  uint3 u3;
  u3.x = a, u3.y = b, u3.z = c;
  return u3;
}
inline uint3 make_uint3(const uint s) { return make_uint3(s, s, s); }
inline uint3 make_uint3(const uint2& a) { return make_uint3(a.x, a.y, 0); }
inline uint3 make_uint3(const uint2& a, const uint s) {
  return make_uint3(a.x, a.y, s);
}
inline uint3 make_uint3(const uint4& a) { return make_uint3(a.x, a.y, a.z); }
inline uint3 make_uint3(const int3& a) {
  return make_uint3(uint(a.x), uint(a.y), uint(a.z));
}
inline float4 make_float4(const float a, const float b, const float c,
                          const float d) {
  float4 f4;
  f4.x = a, f4.y = b, f4.z = c, f4.w = d;
  return f4;
}
inline float4 make_float4(const float s) { return make_float4(s, s, s, s); }
inline float4 make_float4(const float3& a) {
  return make_float4(a.x, a.y, a.z, 0.0f);
}
inline float4 make_float4(const float3& a, const float w) {
  return make_float4(a.x, a.y, a.z, w);
}
inline float4 make_float4(const int3& a, const float w) {
  return make_float4((float)a.x, (float)a.y, (float)a.z, w);
}
inline float4 make_float4(const int4& a) {
  return make_float4(float(a.x), float(a.y), float(a.z), float(a.w));
}
inline float4 make_float4(const uint4& a) {
  return make_float4(float(a.x), float(a.y), float(a.z), float(a.w));
}
inline int4 make_int4(const int a, const int b, const int c, const int d) {
  int4 i4;
  i4.x = a, i4.y = b, i4.z = c, i4.w = d;
  return i4;
}
inline int4 make_int4(const int s) { return make_int4(s, s, s, s); }
inline int4 make_int4(const int3& a) { return make_int4(a.x, a.y, a.z, 0); }
inline int4 make_int4(const int3& a, const int w) {
  return make_int4(a.x, a.y, a.z, w);
}
inline int4 make_int4(const uint4& a) {
  return make_int4(int(a.x), int(a.y), int(a.z), int(a.w));
}
inline int4 make_int4(const float4& a) {
  return make_int4(int(a.x), int(a.y), int(a.z), int(a.w));
}
inline uint4 make_uint4(const uint a, const uint b, const uint c,
                        const uint d) {
  uint4 u4;
  u4.x = a, u4.y = b, u4.z = c, u4.w = d;
  return u4;
}
inline uint4 make_uint4(const uint s) { return make_uint4(s, s, s, s); }
inline uint4 make_uint4(const uint3& a) { return make_uint4(a.x, a.y, a.z, 0); }
inline uint4 make_uint4(const uint3& a, const uint w) {
  return make_uint4(a.x, a.y, a.z, w);
}
inline uint4 make_uint4(const int4& a) {
  return make_uint4(uint(a.x), uint(a.y), uint(a.z), uint(a.w));
}
inline uchar4 make_uchar4(const uchar a, const uchar b, const uchar c,
                          const uchar d) {
  uchar4 c4;
  c4.x = a, c4.y = b, c4.z = c, c4.w = d;
  return c4;
}

inline float2 operator-(const float2& a) { return make_float2(-a.x, -a.y); }
inline int2 operator-(const int2& a) { return make_int2(-a.x, -a.y); }
inline float3 operator-(const float3& a) {
  return make_float3(-a.x, -a.y, -a.z);
}
inline int3 operator-(const int3& a) { return make_int3(-a.x, -a.y, -a.z); }
inline float4 operator-(const float4& a) {
  return make_float4(-a.x, -a.y, -a.z, -a.w);
}
inline int4 operator-(const int4& a) {
  return make_int4(-a.x, -a.y, -a.z, -a.w);
}
inline int2 operator<<(const int2& a, int b) {
  return make_int2(a.x << b, a.y << b);
}
inline int2 operator>>(const int2& a, int b) {
  return make_int2(a.x >> b, a.y >> b);
}
inline int3 operator<<(const int3& a, int b) {
  return make_int3(a.x << b, a.y << b, a.z << b);
}
inline int3 operator>>(const int3& a, int b) {
  return make_int3(a.x >> b, a.y >> b, a.z >> b);
}
inline int4 operator<<(const int4& a, int b) {
  return make_int4(a.x << b, a.y << b, a.z << b, a.w << b);
}
inline int4 operator>>(const int4& a, int b) {
  return make_int4(a.x >> b, a.y >> b, a.z >> b, a.w >> b);
}

inline float2 operator+(const float2& a, const float2& b) {
  return make_float2(a.x + b.x, a.y + b.y);
}
inline float2 operator+(const float2& a, const int2& b) {
  return make_float2(a.x + (float)b.x, a.y + (float)b.y);
}
inline float2 operator+(const float2& a, const uint2& b) {
  return make_float2(a.x + (float)b.x, a.y + (float)b.y);
}
inline float2 operator+(const int2& a, const float2& b) {
  return make_float2((float)a.x + b.x, (float)a.y + b.y);
}
inline float2 operator+(const uint2& a, const float2& b) {
  return make_float2((float)a.x + b.x, (float)a.y + b.y);
}
inline void operator+=(float2& a, const float2& b) {
  a.x += b.x;
  a.y += b.y;
}
inline void operator+=(float2& a, const int2& b) {
  a.x += (float)b.x;
  a.y += (float)b.y;
}
inline void operator+=(float2& a, const uint2& b) {
  a.x += (float)b.x;
  a.y += (float)b.y;
}
inline float2 operator+(const float2& a, float b) {
  return make_float2(a.x + b, a.y + b);
}
inline float2 operator+(const float2& a, int b) {
  return make_float2(a.x + (float)b, a.y + (float)b);
}
inline float2 operator+(const float2& a, uint b) {
  return make_float2(a.x + (float)b, a.y + (float)b);
}
inline float2 operator+(float b, const float2& a) {
  return make_float2(a.x + b, a.y + b);
}
inline void operator+=(float2& a, float b) {
  a.x += b;
  a.y += b;
}
inline void operator+=(float2& a, int b) {
  a.x += (float)b;
  a.y += (float)b;
}
inline void operator+=(float2& a, uint b) {
  a.x += (float)b;
  a.y += (float)b;
}
inline int2 operator+(const int2& a, const int2& b) {
  return make_int2(a.x + b.x, a.y + b.y);
}
inline void operator+=(int2& a, const int2& b) {
  a.x += b.x;
  a.y += b.y;
}
inline int2 operator+(const int2& a, int b) {
  return make_int2(a.x + b, a.y + b);
}
inline int2 operator+(int b, const int2& a) {
  return make_int2(a.x + b, a.y + b);
}
inline void operator+=(int2& a, int b) {
  a.x += b;
  a.y += b;
}
inline uint2 operator+(const uint2& a, const uint2& b) {
  return make_uint2(a.x + b.x, a.y + b.y);
}
inline void operator+=(uint2& a, const uint2& b) {
  a.x += b.x;
  a.y += b.y;
}
inline uint2 operator+(const uint2& a, uint b) {
  return make_uint2(a.x + b, a.y + b);
}
inline uint2 operator+(uint b, const uint2& a) {
  return make_uint2(a.x + b, a.y + b);
}
inline void operator+=(uint2& a, uint b) {
  a.x += b;
  a.y += b;
}
inline float3 operator+(const float3& a, const float3& b) {
  return make_float3(a.x + b.x, a.y + b.y, a.z + b.z);
}
inline float3 operator+(const float3& a, const int3& b) {
  return make_float3(a.x + (float)b.x, a.y + (float)b.y, a.z + (float)b.z);
}
inline float3 operator+(const float3& a, const uint3& b) {
  return make_float3(a.x + (float)b.x, a.y + (float)b.y, a.z + (float)b.z);
}
inline float3 operator+(const int3& a, const float3& b) {
  return make_float3((float)a.x + b.x, (float)a.y + b.y, (float)a.z + b.z);
}
inline float3 operator+(const uint3& a, const float3& b) {
  return make_float3((float)a.x + b.x, (float)a.y + b.y, (float)a.z + b.z);
}
inline void operator+=(float3& a, const float3& b) {
  a.x += b.x;
  a.y += b.y;
  a.z += b.z;
}
inline void operator+=(float3& a, const int3& b) {
  a.x += (float)b.x;
  a.y += (float)b.y;
  a.z += (float)b.z;
}
inline void operator+=(float3& a, const uint3& b) {
  a.x += (float)b.x;
  a.y += (float)b.y;
  a.z += (float)b.z;
}
inline float3 operator+(const float3& a, float b) {
  return make_float3(a.x + b, a.y + b, a.z + b);
}
inline float3 operator+(const float3& a, int b) {
  return make_float3(a.x + (float)b, a.y + (float)b, a.z + (float)b);
}
inline float3 operator+(const float3& a, uint b) {
  return make_float3(a.x + (float)b, a.y + (float)b, a.z + (float)b);
}
inline void operator+=(float3& a, float b) {
  a.x += b;
  a.y += b;
  a.z += b;
}
inline void operator+=(float3& a, int b) {
  a.x += (float)b;
  a.y += (float)b;
  a.z += (float)b;
}
inline void operator+=(float3& a, uint b) {
  a.x += (float)b;
  a.y += (float)b;
  a.z += (float)b;
}
inline int3 operator+(const int3& a, const int3& b) {
  return make_int3(a.x + b.x, a.y + b.y, a.z + b.z);
}
inline void operator+=(int3& a, const int3& b) {
  a.x += b.x;
  a.y += b.y;
  a.z += b.z;
}
inline int3 operator+(const int3& a, int b) {
  return make_int3(a.x + b, a.y + b, a.z + b);
}
inline void operator+=(int3& a, int b) {
  a.x += b;
  a.y += b;
  a.z += b;
}
inline uint3 operator+(const uint3& a, const uint3& b) {
  return make_uint3(a.x + b.x, a.y + b.y, a.z + b.z);
}
inline void operator+=(uint3& a, const uint3& b) {
  a.x += b.x;
  a.y += b.y;
  a.z += b.z;
}
inline uint3 operator+(const uint3& a, uint b) {
  return make_uint3(a.x + b, a.y + b, a.z + b);
}
inline void operator+=(uint3& a, uint b) {
  a.x += b;
  a.y += b;
  a.z += b;
}
inline int3 operator+(int b, const int3& a) {
  return make_int3(a.x + b, a.y + b, a.z + b);
}
inline uint3 operator+(uint b, const uint3& a) {
  return make_uint3(a.x + b, a.y + b, a.z + b);
}
inline float3 operator+(float b, const float3& a) {
  return make_float3(a.x + b, a.y + b, a.z + b);
}
inline float4 operator+(const float4& a, const float4& b) {
  return make_float4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}
inline float4 operator+(const float4& a, const int4& b) {
  return make_float4(a.x + (float)b.x, a.y + (float)b.y, a.z + (float)b.z,
                     a.w + (float)b.w);
}
inline float4 operator+(const float4& a, const uint4& b) {
  return make_float4(a.x + (float)b.x, a.y + (float)b.y, a.z + (float)b.z,
                     a.w + (float)b.w);
}
inline float4 operator+(const int4& a, const float4& b) {
  return make_float4((float)a.x + b.x, (float)a.y + b.y, (float)a.z + b.z,
                     (float)a.w + b.w);
}
inline float4 operator+(const uint4& a, const float4& b) {
  return make_float4((float)a.x + b.x, (float)a.y + b.y, (float)a.z + b.z,
                     (float)a.w + b.w);
}
inline void operator+=(float4& a, const float4& b) {
  a.x += b.x;
  a.y += b.y;
  a.z += b.z;
  a.w += b.w;
}
inline void operator+=(float4& a, const int4& b) {
  a.x += (float)b.x;
  a.y += (float)b.y;
  a.z += (float)b.z;
  a.w += (float)b.w;
}
inline void operator+=(float4& a, const uint4& b) {
  a.x += (float)b.x;
  a.y += (float)b.y;
  a.z += (float)b.z;
  a.w += (float)b.w;
}
inline float4 operator+(const float4& a, float b) {
  return make_float4(a.x + b, a.y + b, a.z + b, a.w + b);
}
inline float4 operator+(const float4& a, int b) {
  return make_float4(a.x + (float)b, a.y + (float)b, a.z + (float)b,
                     a.w + (float)b);
}
inline float4 operator+(const float4& a, uint b) {
  return make_float4(a.x + (float)b, a.y + (float)b, a.z + (float)b,
                     a.w + (float)b);
}
inline float4 operator+(float b, const float4& a) {
  return make_float4(a.x + b, a.y + b, a.z + b, a.w + b);
}
inline void operator+=(float4& a, float b) {
  a.x += b;
  a.y += b;
  a.z += b;
  a.w += b;
}
inline void operator+=(float4& a, int b) {
  a.x += (float)b;
  a.y += (float)b;
  a.z += (float)b;
  a.w += (float)b;
}
inline void operator+=(float4& a, uint b) {
  a.x += (float)b;
  a.y += (float)b;
  a.z += (float)b;
  a.w += (float)b;
}
inline int4 operator+(const int4& a, const int4& b) {
  return make_int4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}
inline void operator+=(int4& a, const int4& b) {
  a.x += b.x;
  a.y += b.y;
  a.z += b.z;
  a.w += b.w;
}
inline int4 operator+(const int4& a, int b) {
  return make_int4(a.x + b, a.y + b, a.z + b, a.w + b);
}
inline int4 operator+(int b, const int4& a) {
  return make_int4(a.x + b, a.y + b, a.z + b, a.w + b);
}
inline void operator+=(int4& a, int b) {
  a.x += b;
  a.y += b;
  a.z += b;
  a.w += b;
}
inline uint4 operator+(const uint4& a, const uint4& b) {
  return make_uint4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}
inline void operator+=(uint4& a, const uint4& b) {
  a.x += b.x;
  a.y += b.y;
  a.z += b.z;
  a.w += b.w;
}
inline uint4 operator+(const uint4& a, uint b) {
  return make_uint4(a.x + b, a.y + b, a.z + b, a.w + b);
}
inline uint4 operator+(uint b, const uint4& a) {
  return make_uint4(a.x + b, a.y + b, a.z + b, a.w + b);
}
inline void operator+=(uint4& a, uint b) {
  a.x += b;
  a.y += b;
  a.z += b;
  a.w += b;
}

inline float2 operator-(const float2& a, const float2& b) {
  return make_float2(a.x - b.x, a.y - b.y);
}
inline float2 operator-(const float2& a, const int2& b) {
  return make_float2(a.x - (float)b.x, a.y - (float)b.y);
}
inline float2 operator-(const float2& a, const uint2& b) {
  return make_float2(a.x - (float)b.x, a.y - (float)b.y);
}
inline float2 operator-(const int2& a, const float2& b) {
  return make_float2((float)a.x - b.x, (float)a.y - b.y);
}
inline float2 operator-(const uint2& a, const float2& b) {
  return make_float2((float)a.x - b.x, (float)a.y - b.y);
}
inline void operator-=(float2& a, const float2& b) {
  a.x -= b.x;
  a.y -= b.y;
}
inline void operator-=(float2& a, const int2& b) {
  a.x -= (float)b.x;
  a.y -= (float)b.y;
}
inline void operator-=(float2& a, const uint2& b) {
  a.x -= (float)b.x;
  a.y -= (float)b.y;
}
inline float2 operator-(const float2& a, float b) {
  return make_float2(a.x - b, a.y - b);
}
inline float2 operator-(const float2& a, int b) {
  return make_float2(a.x - (float)b, a.y - (float)b);
}
inline float2 operator-(const float2& a, uint b) {
  return make_float2(a.x - (float)b, a.y - (float)b);
}
inline float2 operator-(float b, const float2& a) {
  return make_float2(b - a.x, b - a.y);
}
inline void operator-=(float2& a, float b) {
  a.x -= b;
  a.y -= b;
}
inline void operator-=(float2& a, int b) {
  a.x -= (float)b;
  a.y -= (float)b;
}
inline void operator-=(float2& a, uint b) {
  a.x -= (float)b;
  a.y -= (float)b;
}
inline int2 operator-(const int2& a, const int2& b) {
  return make_int2(a.x - b.x, a.y - b.y);
}
inline void operator-=(int2& a, const int2& b) {
  a.x -= b.x;
  a.y -= b.y;
}
inline int2 operator-(const int2& a, int b) {
  return make_int2(a.x - b, a.y - b);
}
inline int2 operator-(int b, const int2& a) {
  return make_int2(b - a.x, b - a.y);
}
inline void operator-=(int2& a, int b) {
  a.x -= b;
  a.y -= b;
}
inline uint2 operator-(const uint2& a, const uint2& b) {
  return make_uint2(a.x - b.x, a.y - b.y);
}
inline void operator-=(uint2& a, const uint2& b) {
  a.x -= b.x;
  a.y -= b.y;
}
inline uint2 operator-(const uint2& a, uint b) {
  return make_uint2(a.x - b, a.y - b);
}
inline uint2 operator-(uint b, const uint2& a) {
  return make_uint2(b - a.x, b - a.y);
}
inline void operator-=(uint2& a, uint b) {
  a.x -= b;
  a.y -= b;
}
inline float3 operator-(const float3& a, const float3& b) {
  return make_float3(a.x - b.x, a.y - b.y, a.z - b.z);
}
inline float3 operator-(const float3& a, const int3& b) {
  return make_float3(a.x - (float)b.x, a.y - (float)b.y, a.z - (float)b.z);
}
inline float3 operator-(const float3& a, const uint3& b) {
  return make_float3(a.x - (float)b.x, a.y - (float)b.y, a.z - (float)b.z);
}
inline float3 operator-(const int3& a, const float3& b) {
  return make_float3((float)a.x - b.x, (float)a.y - b.y, (float)a.z - b.z);
}
inline float3 operator-(const uint3& a, const float3& b) {
  return make_float3((float)a.x - b.x, (float)a.y - b.y, (float)a.z - b.z);
}
inline void operator-=(float3& a, const float3& b) {
  a.x -= b.x;
  a.y -= b.y;
  a.z -= b.z;
}
inline void operator-=(float3& a, const int3& b) {
  a.x -= (float)b.x;
  a.y -= (float)b.y;
  a.z -= (float)b.z;
}
inline void operator-=(float3& a, const uint3& b) {
  a.x -= (float)b.x;
  a.y -= (float)b.y;
  a.z -= (float)b.z;
}
inline float3 operator-(const float3& a, float b) {
  return make_float3(a.x - b, a.y - b, a.z - b);
}
inline float3 operator-(const float3& a, int b) {
  return make_float3(a.x - (float)b, a.y - (float)b, a.z - (float)b);
}
inline float3 operator-(const float3& a, uint b) {
  return make_float3(a.x - (float)b, a.y - (float)b, a.z - (float)b);
}
inline float3 operator-(float b, const float3& a) {
  return make_float3(b - a.x, b - a.y, b - a.z);
}
inline void operator-=(float3& a, float b) {
  a.x -= b;
  a.y -= b;
  a.z -= b;
}
inline void operator-=(float3& a, int b) {
  a.x -= (float)b;
  a.y -= (float)b;
  a.z -= (float)b;
}
inline void operator-=(float3& a, uint b) {
  a.x -= (float)b;
  a.y -= (float)b;
  a.z -= (float)b;
}
inline int3 operator-(const int3& a, const int3& b) {
  return make_int3(a.x - b.x, a.y - b.y, a.z - b.z);
}
inline void operator-=(int3& a, const int3& b) {
  a.x -= b.x;
  a.y -= b.y;
  a.z -= b.z;
}
inline int3 operator-(const int3& a, int b) {
  return make_int3(a.x - b, a.y - b, a.z - b);
}
inline int3 operator-(int b, const int3& a) {
  return make_int3(b - a.x, b - a.y, b - a.z);
}
inline void operator-=(int3& a, int b) {
  a.x -= b;
  a.y -= b;
  a.z -= b;
}
inline uint3 operator-(const uint3& a, const uint3& b) {
  return make_uint3(a.x - b.x, a.y - b.y, a.z - b.z);
}
inline void operator-=(uint3& a, const uint3& b) {
  a.x -= b.x;
  a.y -= b.y;
  a.z -= b.z;
}
inline uint3 operator-(const uint3& a, uint b) {
  return make_uint3(a.x - b, a.y - b, a.z - b);
}
inline uint3 operator-(uint b, const uint3& a) {
  return make_uint3(b - a.x, b - a.y, b - a.z);
}
inline void operator-=(uint3& a, uint b) {
  a.x -= b;
  a.y -= b;
  a.z -= b;
}
inline float4 operator-(const float4& a, const float4& b) {
  return make_float4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}
inline float4 operator-(const float4& a, const int4& b) {
  return make_float4(a.x - (float)b.x, a.y - (float)b.y, a.z - (float)b.z,
                     a.w - (float)b.w);
}
inline float4 operator-(const float4& a, const uint4& b) {
  return make_float4(a.x - (float)b.x, a.y - (float)b.y, a.z - (float)b.z,
                     a.w - (float)b.w);
}
inline float4 operator-(const int4& a, const float4& b) {
  return make_float4((float)a.x - b.x, (float)a.y - b.y, (float)a.z - b.z,
                     (float)a.w - b.w);
}
inline float4 operator-(const uint4& a, const float4& b) {
  return make_float4((float)a.x - b.x, (float)a.y - b.y, (float)a.z - b.z,
                     (float)a.w - b.w);
}
inline void operator-=(float4& a, const float4& b) {
  a.x -= b.x;
  a.y -= b.y;
  a.z -= b.z;
  a.w -= b.w;
}
inline void operator-=(float4& a, const int4& b) {
  a.x -= (float)b.x;
  a.y -= (float)b.y;
  a.z -= (float)b.z;
  a.w -= (float)b.w;
}
inline void operator-=(float4& a, const uint4& b) {
  a.x -= (float)b.x;
  a.y -= (float)b.y;
  a.z -= (float)b.z;
  a.w -= (float)b.w;
}
inline float4 operator-(const float4& a, float b) {
  return make_float4(a.x - b, a.y - b, a.z - b, a.w - b);
}
inline float4 operator-(const float4& a, int b) {
  return make_float4(a.x - (float)b, a.y - (float)b, a.z - (float)b,
                     a.w - (float)b);
}
inline float4 operator-(const float4& a, uint b) {
  return make_float4(a.x - (float)b, a.y - (float)b, a.z - (float)b,
                     a.w - (float)b);
}
inline void operator-=(float4& a, float b) {
  a.x -= b;
  a.y -= b;
  a.z -= b;
  a.w -= b;
}
inline void operator-=(float4& a, int b) {
  a.x -= (float)b;
  a.y -= (float)b;
  a.z -= (float)b;
  a.w -= (float)b;
}
inline void operator-=(float4& a, uint b) {
  a.x -= (float)b;
  a.y -= (float)b;
  a.z -= (float)b;
  a.w -= (float)b;
}
inline int4 operator-(const int4& a, const int4& b) {
  return make_int4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}
inline void operator-=(int4& a, const int4& b) {
  a.x -= b.x;
  a.y -= b.y;
  a.z -= b.z;
  a.w -= b.w;
}
inline int4 operator-(const int4& a, int b) {
  return make_int4(a.x - b, a.y - b, a.z - b, a.w - b);
}
inline int4 operator-(int b, const int4& a) {
  return make_int4(b - a.x, b - a.y, b - a.z, b - a.w);
}
inline void operator-=(int4& a, int b) {
  a.x -= b;
  a.y -= b;
  a.z -= b;
  a.w -= b;
}
inline uint4 operator-(const uint4& a, const uint4& b) {
  return make_uint4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}
inline void operator-=(uint4& a, const uint4& b) {
  a.x -= b.x;
  a.y -= b.y;
  a.z -= b.z;
  a.w -= b.w;
}
inline uint4 operator-(const uint4& a, uint b) {
  return make_uint4(a.x - b, a.y - b, a.z - b, a.w - b);
}
inline uint4 operator-(uint b, const uint4& a) {
  return make_uint4(b - a.x, b - a.y, b - a.z, b - a.w);
}
inline void operator-=(uint4& a, uint b) {
  a.x -= b;
  a.y -= b;
  a.z -= b;
  a.w -= b;
}

inline float2 operator*(const float2& a, const float2& b) {
  return make_float2(a.x * b.x, a.y * b.y);
}
inline void operator*=(float2& a, const float2& b) {
  a.x *= b.x;
  a.y *= b.y;
}
inline float2 operator*(const float2& a, float b) {
  return make_float2(a.x * b, a.y * b);
}
inline float2 operator*(float b, const float2& a) {
  return make_float2(b * a.x, b * a.y);
}
inline void operator*=(float2& a, float b) {
  a.x *= b;
  a.y *= b;
}
inline int2 operator*(const int2& a, const int2& b) {
  return make_int2(a.x * b.x, a.y * b.y);
}
inline void operator*=(int2& a, const int2& b) {
  a.x *= b.x;
  a.y *= b.y;
}
inline int2 operator*(const int2& a, int b) {
  return make_int2(a.x * b, a.y * b);
}
inline int2 operator*(int b, const int2& a) {
  return make_int2(b * a.x, b * a.y);
}
inline void operator*=(int2& a, int b) {
  a.x *= b;
  a.y *= b;
}
inline uint2 operator*(const uint2& a, const uint2& b) {
  return make_uint2(a.x * b.x, a.y * b.y);
}
inline void operator*=(uint2& a, const uint2& b) {
  a.x *= b.x;
  a.y *= b.y;
}
inline uint2 operator*(const uint2& a, uint b) {
  return make_uint2(a.x * b, a.y * b);
}
inline uint2 operator*(uint b, const uint2& a) {
  return make_uint2(b * a.x, b * a.y);
}
inline void operator*=(uint2& a, uint b) {
  a.x *= b;
  a.y *= b;
}
inline float3 operator*(const float3& a, const float3& b) {
  return make_float3(a.x * b.x, a.y * b.y, a.z * b.z);
}
inline void operator*=(float3& a, const float3& b) {
  a.x *= b.x;
  a.y *= b.y;
  a.z *= b.z;
}
inline float3 operator*(const float3& a, float b) {
  return make_float3(a.x * b, a.y * b, a.z * b);
}
inline float3 operator*(float b, const float3& a) {
  return make_float3(b * a.x, b * a.y, b * a.z);
}
inline void operator*=(float3& a, float b) {
  a.x *= b;
  a.y *= b;
  a.z *= b;
}
inline int3 operator*(const int3& a, const int3& b) {
  return make_int3(a.x * b.x, a.y * b.y, a.z * b.z);
}
inline void operator*=(int3& a, const int3& b) {
  a.x *= b.x;
  a.y *= b.y;
  a.z *= b.z;
}
inline int3 operator*(const int3& a, int b) {
  return make_int3(a.x * b, a.y * b, a.z * b);
}
inline int3 operator*(int b, const int3& a) {
  return make_int3(b * a.x, b * a.y, b * a.z);
}
inline void operator*=(int3& a, int b) {
  a.x *= b;
  a.y *= b;
  a.z *= b;
}
inline uint3 operator*(const uint3& a, const uint3& b) {
  return make_uint3(a.x * b.x, a.y * b.y, a.z * b.z);
}
inline void operator*=(uint3& a, const uint3& b) {
  a.x *= b.x;
  a.y *= b.y;
  a.z *= b.z;
}
inline uint3 operator*(const uint3& a, uint b) {
  return make_uint3(a.x * b, a.y * b, a.z * b);
}
inline uint3 operator*(uint b, const uint3& a) {
  return make_uint3(b * a.x, b * a.y, b * a.z);
}
inline void operator*=(uint3& a, uint b) {
  a.x *= b;
  a.y *= b;
  a.z *= b;
}
inline float4 operator*(const float4& a, const float4& b) {
  return make_float4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
}
inline void operator*=(float4& a, const float4& b) {
  a.x *= b.x;
  a.y *= b.y;
  a.z *= b.z;
  a.w *= b.w;
}
inline float4 operator*(const float4& a, float b) {
  return make_float4(a.x * b, a.y * b, a.z * b, a.w * b);
}
inline float4 operator*(float b, const float4& a) {
  return make_float4(b * a.x, b * a.y, b * a.z, b * a.w);
}
inline void operator*=(float4& a, float b) {
  a.x *= b;
  a.y *= b;
  a.z *= b;
  a.w *= b;
}
inline int4 operator*(const int4& a, const int4& b) {
  return make_int4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
}
inline void operator*=(int4& a, const int4& b) {
  a.x *= b.x;
  a.y *= b.y;
  a.z *= b.z;
  a.w *= b.w;
}
inline int4 operator*(const int4& a, int b) {
  return make_int4(a.x * b, a.y * b, a.z * b, a.w * b);
}
inline int4 operator*(int b, const int4& a) {
  return make_int4(b * a.x, b * a.y, b * a.z, b * a.w);
}
inline void operator*=(int4& a, int b) {
  a.x *= b;
  a.y *= b;
  a.z *= b;
  a.w *= b;
}
inline uint4 operator*(const uint4& a, const uint4& b) {
  return make_uint4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
}
inline void operator*=(uint4& a, const uint4& b) {
  a.x *= b.x;
  a.y *= b.y;
  a.z *= b.z;
  a.w *= b.w;
}
inline uint4 operator*(const uint4& a, uint b) {
  return make_uint4(a.x * b, a.y * b, a.z * b, a.w * b);
}
inline uint4 operator*(uint b, const uint4& a) {
  return make_uint4(b * a.x, b * a.y, b * a.z, b * a.w);
}
inline void operator*=(uint4& a, uint b) {
  a.x *= b;
  a.y *= b;
  a.z *= b;
  a.w *= b;
}

inline float2 operator/(const float2& a, const float2& b) {
  return make_float2(a.x / b.x, a.y / b.y);
}
inline void operator/=(float2& a, const float2& b) {
  a.x /= b.x;
  a.y /= b.y;
}
inline float2 operator/(const float2& a, float b) {
  return make_float2(a.x / b, a.y / b);
}
inline void operator/=(float2& a, float b) {
  a.x /= b;
  a.y /= b;
}
inline float2 operator/(float b, const float2& a) {
  return make_float2(b / a.x, b / a.y);
}
inline float3 operator/(const float3& a, const float3& b) {
  return make_float3(a.x / b.x, a.y / b.y, a.z / b.z);
}
inline void operator/=(float3& a, const float3& b) {
  a.x /= b.x;
  a.y /= b.y;
  a.z /= b.z;
}
inline float3 operator/(const float3& a, float b) {
  return make_float3(a.x / b, a.y / b, a.z / b);
}
inline void operator/=(float3& a, float b) {
  a.x /= b;
  a.y /= b;
  a.z /= b;
}
inline float3 operator/(float b, const float3& a) {
  return make_float3(b / a.x, b / a.y, b / a.z);
}
inline float4 operator/(const float4& a, const float4& b) {
  return make_float4(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w);
}
inline void operator/=(float4& a, const float4& b) {
  a.x /= b.x;
  a.y /= b.y;
  a.z /= b.z;
  a.w /= b.w;
}
inline float4 operator/(const float4& a, float b) {
  return make_float4(a.x / b, a.y / b, a.z / b, a.w / b);
}
inline void operator/=(float4& a, float b) {
  a.x /= b;
  a.y /= b;
  a.z /= b;
  a.w /= b;
}
inline float4 operator/(float b, const float4& a) {
  return make_float4(b / a.x, b / a.y, b / a.z, b / a.w);
}

inline float2 fminf(const float2& a, const float2& b) {
  return make_float2(fminf(a.x, b.x), fminf(a.y, b.y));
}
inline float3 fminf(const float3& a, const float3& b) {
  return make_float3(fminf(a.x, b.x), fminf(a.y, b.y), fminf(a.z, b.z));
}
inline float4 fminf(const float4& a, const float4& b) {
  return make_float4(fminf(a.x, b.x), fminf(a.y, b.y), fminf(a.z, b.z),
                     fminf(a.w, b.w));
}
inline int2 min(const int2& a, const int2& b) {
  return make_int2(std::min(a.x, b.x), std::min(a.y, b.y));
}
inline int3 min(const int3& a, const int3& b) {
  return make_int3(std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z));
}
inline int4 min(const int4& a, const int4& b) {
  return make_int4(std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z),
                   std::min(a.w, b.w));
}
inline uint2 min(const uint2& a, const uint2& b) {
  return make_uint2(std::min(a.x, b.x), std::min(a.y, b.y));
}
inline uint3 min(const uint3& a, const uint3& b) {
  return make_uint3(std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z));
}
inline uint4 min(const uint4& a, const uint4& b) {
  return make_uint4(std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z),
                    std::min(a.w, b.w));
}

inline float2 fmaxf(const float2& a, const float2& b) {
  return make_float2(fmaxf(a.x, b.x), fmaxf(a.y, b.y));
}
inline float3 fmaxf(const float3& a, const float3& b) {
  return make_float3(fmaxf(a.x, b.x), fmaxf(a.y, b.y), fmaxf(a.z, b.z));
}
inline float4 fmaxf(const float4& a, const float4& b) {
  return make_float4(fmaxf(a.x, b.x), fmaxf(a.y, b.y), fmaxf(a.z, b.z),
                     fmaxf(a.w, b.w));
}
inline int2 max(const int2& a, const int2& b) {
  return make_int2(std::max(a.x, b.x), std::max(a.y, b.y));
}
inline int3 max(const int3& a, const int3& b) {
  return make_int3(std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z));
}
inline int4 max(const int4& a, const int4& b) {
  return make_int4(std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z),
                   std::max(a.w, b.w));
}
inline uint2 max(const uint2& a, const uint2& b) {
  return make_uint2(std::max(a.x, b.x), std::max(a.y, b.y));
}
inline uint3 max(const uint3& a, const uint3& b) {
  return make_uint3(std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z));
}
inline uint4 max(const uint4& a, const uint4& b) {
  return make_uint4(std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z),
                    std::max(a.w, b.w));
}

inline float lerp(float a, float b, float t) { return a + t * (b - a); }
inline float2 lerp(const float2& a, const float2& b, float t) {
  return a + t * (b - a);
}
inline float3 lerp(const float3& a, const float3& b, float t) {
  return a + t * (b - a);
}
inline float4 lerp(const float4& a, const float4& b, float t) {
  return a + t * (b - a);
}

inline float clamp(float f, float a, float b) { return fmaxf(a, fminf(f, b)); }
inline int clamp(int f, int a, int b) { return std::max(a, std::min(f, b)); }
inline uint clamp(uint f, uint a, uint b) {
  return std::max(a, std::min(f, b));
}
inline float2 clamp(const float2& v, float a, float b) {
  return make_float2(clamp(v.x, a, b), clamp(v.y, a, b));
}
inline float2 clamp(const float2& v, const float2& a, const float2& b) {
  return make_float2(clamp(v.x, a.x, b.x), clamp(v.y, a.y, b.y));
}
inline float3 clamp(const float3& v, float a, float b) {
  return make_float3(clamp(v.x, a, b), clamp(v.y, a, b), clamp(v.z, a, b));
}
inline float3 clamp(const float3& v, const float3& a, const float3& b) {
  return make_float3(clamp(v.x, a.x, b.x), clamp(v.y, a.y, b.y),
                     clamp(v.z, a.z, b.z));
}
inline float4 clamp(const float4& v, float a, float b) {
  return make_float4(clamp(v.x, a, b), clamp(v.y, a, b), clamp(v.z, a, b),
                     clamp(v.w, a, b));
}
inline float4 clamp(const float4& v, const float4& a, const float4& b) {
  return make_float4(clamp(v.x, a.x, b.x), clamp(v.y, a.y, b.y),
                     clamp(v.z, a.z, b.z), clamp(v.w, a.w, b.w));
}
inline int2 clamp(const int2& v, int a, int b) {
  return make_int2(clamp(v.x, a, b), clamp(v.y, a, b));
}
inline int2 clamp(const int2& v, const int2& a, const int2& b) {
  return make_int2(clamp(v.x, a.x, b.x), clamp(v.y, a.y, b.y));
}
inline int3 clamp(const int3& v, int a, int b) {
  return make_int3(clamp(v.x, a, b), clamp(v.y, a, b), clamp(v.z, a, b));
}
inline int3 clamp(const int3& v, const int3& a, const int3& b) {
  return make_int3(clamp(v.x, a.x, b.x), clamp(v.y, a.y, b.y),
                   clamp(v.z, a.z, b.z));
}
inline int4 clamp(const int4& v, int a, int b) {
  return make_int4(clamp(v.x, a, b), clamp(v.y, a, b), clamp(v.z, a, b),
                   clamp(v.w, a, b));
}
inline int4 clamp(const int4& v, const int4& a, const int4& b) {
  return make_int4(clamp(v.x, a.x, b.x), clamp(v.y, a.y, b.y),
                   clamp(v.z, a.z, b.z), clamp(v.w, a.w, b.w));
}
inline uint2 clamp(const uint2& v, uint a, uint b) {
  return make_uint2(clamp(v.x, a, b), clamp(v.y, a, b));
}
inline uint2 clamp(const uint2& v, const uint2& a, const uint2& b) {
  return make_uint2(clamp(v.x, a.x, b.x), clamp(v.y, a.y, b.y));
}
inline uint3 clamp(const uint3& v, uint a, uint b) {
  return make_uint3(clamp(v.x, a, b), clamp(v.y, a, b), clamp(v.z, a, b));
}
inline uint3 clamp(const uint3& v, const uint3& a, const uint3& b) {
  return make_uint3(clamp(v.x, a.x, b.x), clamp(v.y, a.y, b.y),
                    clamp(v.z, a.z, b.z));
}
inline uint4 clamp(const uint4& v, uint a, uint b) {
  return make_uint4(clamp(v.x, a, b), clamp(v.y, a, b), clamp(v.z, a, b),
                    clamp(v.w, a, b));
}
inline uint4 clamp(const uint4& v, const uint4& a, const uint4& b) {
  return make_uint4(clamp(v.x, a.x, b.x), clamp(v.y, a.y, b.y),
                    clamp(v.z, a.z, b.z), clamp(v.w, a.w, b.w));
}

inline float dot(const float2& a, const float2& b) {
  return a.x * b.x + a.y * b.y;
}
inline float dot(const float3& a, const float3& b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}
inline float dot(const float4& a, const float4& b) {
  return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}
inline int dot(const int2& a, const int2& b) { return a.x * b.x + a.y * b.y; }
inline int dot(const int3& a, const int3& b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}
inline int dot(const int4& a, const int4& b) {
  return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}
inline uint dot(const uint2& a, const uint2& b) {
  return a.x * b.x + a.y * b.y;
}
inline uint dot(const uint3& a, const uint3& b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}
inline uint dot(const uint4& a, const uint4& b) {
  return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

inline float sqrLength(const float2& v) { return dot(v, v); }
inline float sqrLength(const float3& v) { return dot(v, v); }
inline float sqrLength(const float4& v) { return dot(v, v); }

inline float length(const float2& v) { return sqrtf(dot(v, v)); }
inline float length(const float3& v) { return sqrtf(dot(v, v)); }
inline float length(const float4& v) { return sqrtf(dot(v, v)); }

inline float length(const int2& v) { return sqrtf((float)dot(v, v)); }
inline float length(const int3& v) { return sqrtf((float)dot(v, v)); }
inline float length(const int4& v) { return sqrtf((float)dot(v, v)); }

inline float2 normalize(const float2& v) {
  float invLen = rsqrtf(dot(v, v));
  return v * invLen;
}
inline float3 normalize(const float3& v) {
  float invLen = rsqrtf(dot(v, v));
  return v * invLen;
}
inline float4 normalize(const float4& v) {
  float invLen = rsqrtf(dot(v, v));
  return v * invLen;
}

inline uint dominantAxis(const float2& v) {
  float x = fabs(v.x), y = fabs(v.y);
  return x > y ? 0 : 1;
}  // for coherent grid traversal
inline uint dominantAxis(const float3& v) {
  float x = fabs(v.x), y = fabs(v.y), z = fabs(v.z);
  float m = std::max(std::max(x, y), z);
  return m == x ? 0 : (m == y ? 1 : 2);
}

inline float2 floorf(const float2& v) {
  return make_float2(floorf(v.x), floorf(v.y));
}
inline float3 floorf(const float3& v) {
  return make_float3(floorf(v.x), floorf(v.y), floorf(v.z));
}
inline float4 floorf(const float4& v) {
  return make_float4(floorf(v.x), floorf(v.y), floorf(v.z), floorf(v.w));
}

inline float fracf(float v) { return v - floorf(v); }
inline float2 fracf(const float2& v) {
  return make_float2(fracf(v.x), fracf(v.y));
}
inline float3 fracf(const float3& v) {
  return make_float3(fracf(v.x), fracf(v.y), fracf(v.z));
}
inline float4 fracf(const float4& v) {
  return make_float4(fracf(v.x), fracf(v.y), fracf(v.z), fracf(v.w));
}

inline float2 fmodf(const float2& a, const float2& b) {
  return make_float2(fmodf(a.x, b.x), fmodf(a.y, b.y));
}
inline float3 fmodf(const float3& a, const float3& b) {
  return make_float3(fmodf(a.x, b.x), fmodf(a.y, b.y), fmodf(a.z, b.z));
}
inline float4 fmodf(const float4& a, const float4& b) {
  return make_float4(fmodf(a.x, b.x), fmodf(a.y, b.y), fmodf(a.z, b.z),
                     fmodf(a.w, b.w));
}

inline float2 fabs(const float2& v) {
  return make_float2(fabs(v.x), fabs(v.y));
}
inline float3 fabs(const float3& v) {
  return make_float3(fabs(v.x), fabs(v.y), fabs(v.z));
}
inline float4 fabs(const float4& v) {
  return make_float4(fabs(v.x), fabs(v.y), fabs(v.z), fabs(v.w));
}
inline int2 abs(const int2& v) { return make_int2(abs(v.x), abs(v.y)); }
inline int3 abs(const int3& v) {
  return make_int3(abs(v.x), abs(v.y), abs(v.z));
}
inline int4 abs(const int4& v) {
  return make_int4(abs(v.x), abs(v.y), abs(v.z), abs(v.w));
}

inline float3 reflect(const float3& i, const float3& n) {
  return i - 2.0f * n * dot(n, i);
}

inline float3 cross(const float3& a, const float3& b) {
  return make_float3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z,
                     a.x * b.y - a.y * b.x);
}

inline float smoothstep(float a, float b, float x) {
  float y = clamp((x - a) / (b - a), 0.0f, 1.0f);
  return (y * y * (3.0f - (2.0f * y)));
}
inline float2 smoothstep(float2 a, float2 b, float2 x) {
  float2 y = clamp((x - a) / (b - a), 0.0f, 1.0f);
  return (y * y * (make_float2(3.0f) - (make_float2(2.0f) * y)));
}
inline float3 smoothstep(float3 a, float3 b, float3 x) {
  float3 y = clamp((x - a) / (b - a), 0.0f, 1.0f);
  return (y * y * (make_float3(3.0f) - (make_float3(2.0f) * y)));
}
inline float4 smoothstep(float4 a, float4 b, float4 x) {
  float4 y = clamp((x - a) / (b - a), 0.0f, 1.0f);
  return (y * y * (make_float4(3.0f) - (make_float4(2.0f) * y)));
}

struct Timer {
  Timer() { reset(); }
  float elapsed() const {
    using namespace std;
    chrono::high_resolution_clock::time_point t2 =
        chrono::high_resolution_clock::now();
    chrono::duration<double> time_span =
        chrono::duration_cast<chrono::duration<double>>(t2 - start);
    return (float)time_span.count();
  }
  void reset() { start = std::chrono::high_resolution_clock::now(); }
  std::chrono::high_resolution_clock::time_point start;
};

//==============================================================================

// enable the use of SSE in the AABB intersection function
#define USE_SSE

// triangle count
#define N 12582  // hardcoded for the unity vehicle mesh

// forward declarations
void Subdivide(uint nodeIdx);
void UpdateNodeBounds(uint nodeIdx);

// minimal structs
struct Tri {
  float3 vertex0, vertex1, vertex2;
  float3 centroid;
};
struct BVHNode {
  union {
    struct {
      float3 aabbMin;
      uint leftFirst;
    };
    __m128 aabbMin4;
  };
  union {
    struct {
      float3 aabbMax;
      uint triCount;
    };
    __m128 aabbMax4;
  };
  bool isLeaf() { return triCount > 0; }

  std::string print() {
    if (isLeaf()) {
      return std::format("leaf: [{},{})", leftFirst, leftFirst + triCount);
    } else {
      return std::format("node: ({},{})", leftFirst, leftFirst + 1);
    }
  }
};

struct aabb {
  float3 bmin = 1e30f, bmax = -1e30f;
  void grow(float3 p) {
    bmin = fminf(bmin, p);
    bmax = fmaxf(bmax, p);
  }
  float area() {
    float3 e = bmax - bmin;  // box extent
    return e.x * e.y + e.y * e.z + e.z * e.x;
  }
};
struct alignas(64) Ray {
  Ray() { O4 = D4 = rD4 = _mm_set1_ps(1); }
  union {
    struct {
      float3 O;
      float dummy1;
    };
    __m128 O4;
  };
  union {
    struct {
      float3 D;
      float dummy2;
    };
    __m128 D4;
  };
  union {
    struct {
      float3 rD;
      float dummy3;
    };
    __m128 rD4;
  };
  float t = 1e30f;
};

// application data
Tri tri[N];
uint triIdx[N];
BVHNode* bvhNode = 0;
uint rootNodeIdx = 0, nodesUsed = 2;

// functions

void IntersectTri(Ray& ray, const Tri& tri) {
  const float3 edge1 = tri.vertex1 - tri.vertex0;
  const float3 edge2 = tri.vertex2 - tri.vertex0;
  const float3 h = cross(ray.D, edge2);
  const float a = dot(edge1, h);
  if (a > -0.0001f && a < 0.0001f) return;  // ray parallel to triangle
  const float f = 1 / a;
  const float3 s = ray.O - tri.vertex0;
  const float u = f * dot(s, h);
  if (u < 0 || u > 1) return;
  const float3 q = cross(s, edge1);
  const float v = f * dot(ray.D, q);
  if (v < 0 || u + v > 1) return;
  const float t = f * dot(edge2, q);
  if (t > 0.0001f) ray.t = std::min(ray.t, t);
}

inline float IntersectAABB(const Ray& ray, const float3 bmin,
                           const float3 bmax) {
  float tx1 = (bmin.x - ray.O.x) * ray.rD.x,
        tx2 = (bmax.x - ray.O.x) * ray.rD.x;
  float tmin = std::min(tx1, tx2), tmax = std::max(tx1, tx2);
  float ty1 = (bmin.y - ray.O.y) * ray.rD.y,
        ty2 = (bmax.y - ray.O.y) * ray.rD.y;
  tmin = std::max(tmin, std::min(ty1, ty2)),
  tmax = std::min(tmax, std::max(ty1, ty2));
  float tz1 = (bmin.z - ray.O.z) * ray.rD.z,
        tz2 = (bmax.z - ray.O.z) * ray.rD.z;
  tmin = std::max(tmin, std::min(tz1, tz2)),
  tmax = std::min(tmax, std::max(tz1, tz2));
  if (tmax >= tmin && tmin < ray.t && tmax > 0)
    return tmin;
  else
    return 1e30f;
}

struct m128_helper {
  union {
    __m128 m128;
    float m128_f32[4];
  };

  float min() const {
    return std::min(m128_f32[0], std::min(m128_f32[1], m128_f32[2]));
  }

  float max() const {
    return std::max(m128_f32[0], std::max(m128_f32[1], m128_f32[2]));
  }
};

float IntersectAABB_SSE(const Ray& ray, const __m128& bmin4,
                        const __m128& bmax4) {
  static __m128 mask4 = _mm_cmpeq_ps(_mm_setzero_ps(), _mm_set_ps(1, 0, 0, 0));
  __m128 t1 = _mm_mul_ps(_mm_sub_ps(_mm_and_ps(bmin4, mask4), ray.O4), ray.rD4);
  __m128 t2 = _mm_mul_ps(_mm_sub_ps(_mm_and_ps(bmax4, mask4), ray.O4), ray.rD4);
  __m128 vmax4 = _mm_max_ps(t1, t2), vmin4 = _mm_min_ps(t1, t2);

  float tmax = m128_helper{vmax4}.min();
  float tmin = m128_helper{vmin4}.max();
  if (tmax >= tmin && tmin < ray.t && tmax > 0)
    return tmin;
  else
    return 1e30f;
}

void IntersectBVH(Ray& ray) {
  BVHNode *node = &bvhNode[rootNodeIdx], *stack[64];
  uint stackPtr = 0;
  while (1) {
    if (node->isLeaf()) {
      for (uint i = 0; i < node->triCount; i++)
        IntersectTri(ray, tri[triIdx[node->leftFirst + i]]);
      if (stackPtr == 0)
        break;
      else
        node = stack[--stackPtr];
      continue;
    }
    BVHNode* child1 = &bvhNode[node->leftFirst];
    BVHNode* child2 = &bvhNode[node->leftFirst + 1];
#ifdef USE_SSEE
    float dist1 = IntersectAABB_SSE(ray, child1->aabbMin4, child1->aabbMax4);
    float dist2 = IntersectAABB_SSE(ray, child2->aabbMin4, child2->aabbMax4);
#else
    float dist1 = IntersectAABB(ray, child1->aabbMin, child1->aabbMax);
    float dist2 = IntersectAABB(ray, child2->aabbMin, child2->aabbMax);
#endif
    if (dist1 > dist2) {
      std::swap(dist1, dist2);
      std::swap(child1, child2);
    }
    if (dist1 == 1e30f) {
      if (stackPtr == 0)
        break;
      else
        node = stack[--stackPtr];
    } else {
      node = child1;
      if (dist2 != 1e30f) stack[stackPtr++] = child2;
    }
  }
}

void BuildBVH() {
  Timer t;
  // create the BVH node pool
  //   bvhNode = (BVHNode*)aligned_malloc(sizeof(BVHNode) * N * 2, 64);
  bvhNode = (BVHNode*)aligned_alloc(64, sizeof(BVHNode) * N * 2);
  // populate triangle index array
  for (int i = 0; i < N; i++) triIdx[i] = i;
  // calculate triangle centroids for partitioning
  for (int i = 0; i < N; i++)
    tri[i].centroid =
        (tri[i].vertex0 + tri[i].vertex1 + tri[i].vertex2) * 0.3333f;
  // assign all triangles to root node
  BVHNode& root = bvhNode[rootNodeIdx];
  root.leftFirst = 0, root.triCount = N;
  UpdateNodeBounds(rootNodeIdx);
  // subdivide recursively

  Subdivide(rootNodeIdx);
  std::println("BVH ({} nodes) constructed in {}ms.", nodesUsed,
               t.elapsed() * 1000);
}

void UpdateNodeBounds(uint nodeIdx) {
  BVHNode& node = bvhNode[nodeIdx];
  node.aabbMin = float3(1e30f);
  node.aabbMax = float3(-1e30f);
  for (uint first = node.leftFirst, i = 0; i < node.triCount; i++) {
    uint leafTriIdx = triIdx[first + i];
    Tri& leafTri = tri[leafTriIdx];
    node.aabbMin = fminf(node.aabbMin, leafTri.vertex0);
    node.aabbMin = fminf(node.aabbMin, leafTri.vertex1);
    node.aabbMin = fminf(node.aabbMin, leafTri.vertex2);
    node.aabbMax = fmaxf(node.aabbMax, leafTri.vertex0);
    node.aabbMax = fmaxf(node.aabbMax, leafTri.vertex1);
    node.aabbMax = fmaxf(node.aabbMax, leafTri.vertex2);
  }
}

float EvaluateSAH(BVHNode& node, int axis, float pos, int idx) {
  // determine triangle counts and bounds for this split candidate
  aabb leftBox, rightBox;
  int leftCount = 0, rightCount = 0;
  for (uint i = 0; i < node.triCount; i++) {
    Tri& triangle = tri[triIdx[node.leftFirst + i]];
    if (triangle.centroid[axis] < pos) {
      leftCount++;
      leftBox.grow(triangle.vertex0);
      leftBox.grow(triangle.vertex1);
      leftBox.grow(triangle.vertex2);
    } else {
      rightCount++;
      rightBox.grow(triangle.vertex0);
      rightBox.grow(triangle.vertex1);
      rightBox.grow(triangle.vertex2);
    }
  }
  float cost = leftCount * leftBox.area() + rightCount * rightBox.area();

  return cost > 0 ? cost : 1e30f;
}

void Subdivide(uint nodeIdx) {
  // terminate recursion
  BVHNode& node = bvhNode[nodeIdx];
  // determine split axis using SAH
  int bestAxis = -1;
  float bestPos = 0, bestCost = 1e30f;
  int idx;
  for (int axis = 0; axis < 3; axis++)
    for (uint i = 0; i < node.triCount; i++) {
      Tri& triangle = tri[triIdx[node.leftFirst + i]];
      float candidatePos = triangle.centroid[axis];
      float cost = EvaluateSAH(node, axis, candidatePos, node.leftFirst + i);
      if (cost < bestCost)
        bestPos = candidatePos, bestAxis = axis, bestCost = cost,
        idx = node.leftFirst + i;
    }

  int axis = bestAxis;
  float splitPos = bestPos;
  float3 e = node.aabbMax - node.aabbMin;  // extent of parent
  float parentArea = e.x * e.y + e.y * e.z + e.z * e.x;
  float parentCost = node.triCount * parentArea;
  if (bestCost >= parentCost) return;
  // in-place partition
  int i = node.leftFirst;
  int j = i + node.triCount - 1;

  while (i <= j) {
    if (tri[triIdx[i]].centroid[axis] < splitPos)
      i++;
    else
      std::swap(triIdx[i], triIdx[j--]);
  }
  // abort split if one of the sides is empty
  int leftCount = i - node.leftFirst;
  if (leftCount == 0 || leftCount == node.triCount) return;
  // create child nodes
  int leftChildIdx = nodesUsed++;
  int rightChildIdx = nodesUsed++;
  bvhNode[leftChildIdx].leftFirst = node.leftFirst;
  bvhNode[leftChildIdx].triCount = leftCount;
  bvhNode[rightChildIdx].leftFirst = i;
  bvhNode[rightChildIdx].triCount = node.triCount - leftCount;
  auto old = node;
  node.leftFirst = leftChildIdx;
  node.triCount = 0;

  UpdateNodeBounds(leftChildIdx);
  UpdateNodeBounds(rightChildIdx);
  // recurse
  Subdivide(leftChildIdx);
  Subdivide(rightChildIdx);
}

void unity_model() {
  Timer t;
  FILE* file = fopen("unity.tri", "r");
  float a, b, c, d, e, f, g, h, i;
  for (int t = 0; t < N; t++) {
    fscanf(file, "%f %f %f %f %f %f %f %f %f\n", &a, &b, &c, &d, &e, &f, &g, &h,
           &i);
    tri[t].vertex0 = float3(a, b, c);
    tri[t].vertex1 = float3(d, e, f);
    tri[t].vertex2 = float3(g, h, i);
  }
  fclose(file);

  float elapsed = t.elapsed() * 1000;
  printf("%s cost: %.2fms\n", __FUNCTION__, elapsed);

  // construct the BVH
  BuildBVH();
}

#define SCRHEIGHT 640
#define SCRWIDTH 640

int main(int argc, char** argv) {
  unity_model();
  float total_elapsed = 0;
  int cnt = 0;
  run("bvh reference", 640, 640, [&](Surface& canvas) {
    Timer t;
    canvas.Clear(0);

    float3 p0(-2.5f, 0.8f, -0.5f);
    float3 p1(-0.5f, 0.8f, -0.5f);
    float3 p2(-2.5f, -1.2f, -0.5f);

    float3 cam_pos(-1.5f, -0.2f, -2.5f);

    int len = canvas.height * canvas.width;
    std::vector<int> indices(len);
    std::iota(indices.begin(), indices.end(), 0);
    std::for_each_n(std::execution::par, indices.begin(), len, [&](int& i) {
      int x = i % canvas.width;
      int y = i / canvas.width;
      float u = x / float(canvas.width);
      float v = y / float(canvas.height);
      float3 pixel_pos = p0 + (p1 - p0) * u + (p2 - p0) * v;

      Ray ray;
      ray.O = float3(-1.5f, -0.2f, -2.5f);
      ray.D = normalize(pixel_pos - ray.O), ray.t = 1e30f;
      ray.rD = float3(1 / ray.D.x, 1 / ray.D.y, 1 / ray.D.z);
      IntersectBVH(ray);
      uint c = 500 - (int)(ray.t * 42);
      if (ray.t < 1e30f) canvas.Plot(x + u, y + v, c * 0x10101);
    });

    float elapsed = t.elapsed() * 1000;
    printf("tracing time: %.2fms (%5.2fK rays/s)\n", elapsed,
           sqr(630) / elapsed);
    total_elapsed += elapsed;
    cnt += 1;
  });

  printf("average time: %.2fms\n", total_elapsed / cnt);

  return EXIT_SUCCESS;
}