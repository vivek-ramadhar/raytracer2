//
// Created by vivek on 10/19/2025.
//

#pragma once
#include <algorithm>

#ifndef __min
#define __min(a, b) std::min(a, b)
#endif

#ifndef __max
#define __max(a, b) std::max(a, b)
#endif

#ifndef MATH_H
#define MATH_H
#include <cmath>

// ALIGN macro tells the compiler at type definition and variable declaration
// how to lay out the memory so it is aligned to some byte count
#ifdef _MSC_VER
#define ALIGN(x) __declspec(align(x))
#define MALLOC64(x) ((x) == 0 ? 0 : _aligned_malloc((x), 64))
#define FREE64(x) _aligned_free(x)
#else
#define ALIGN(x) __attribute__((aligned(x)))
#include <cstdlib>
#define MALLOC64(n)                                                            \
  ((n) == 0 ? nullptr : std::aligned_alloc(64, ((n) + 63) & ~size_t(63)))
#define FREE64(p) std::free(p)
#endif

typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned short ushort;

#ifdef _MSC_VER
typedef unsigned char BYTE;  // for freeimage.h
typedef unsigned short WORD; // for freeimage.h
typedef unsigned long DWORD; // for freeimage.h
typedef int BOOL;            // for freeimage.h
#endif

// Marsaglias xor32
static uint seed = 0x12345678;
inline uint RandomUInt() {
  seed ^= seed << 13;
  seed ^= seed >> 17;
  seed ^= seed << 5;
  return seed;
}

inline uint RandomUInt(uint &seed) {
  seed ^= seed << 13;
  seed ^= seed >> 17;
  seed ^= seed << 5;
  return seed;
}

inline float random_float() { return RandomUInt() * 2.3283064365387e-10f; }
inline float random_float(uint &seed) {
  return RandomUInt(seed) * 2.3283064365387e-10f;
}
inline float rand(float range) { return random_float() * range; }

struct float3;
struct ALIGN(16) float4 {
  float4() = default;
  float4(const float a, const float b, const float c, const float d)
      : x(a), y(b), z(c), w(d) {}
  float4(const int a) : x(a), y(a), z(a), w(a) {}
  float4(const float3 &a, const float d);
  float4(const float3 &a);
  union {
    struct {
      float x, y, z, w;
    };
    float cell[4];
  };
  float &operator[](const int n) { return cell[n]; }
};

struct ALIGN(16) float3 {
  float3() = default;
  // union here defines two different ways to access the same underlying memory
  union {
    struct {
      float x, y, z;
    };
    float cell[3];
  };
  float3(const float a, const float b, const float c) : x(a), y(b), z(c) {}
  float3(const float a) : x(a), y(a), z(a) {}
  float &operator[](const int n) { return cell[n]; }
};

inline float3 make_float3(const float &a, const float &b, const float &c) {
  float3 f3;
  f3.x = a, f3.y = b, f3.z = c;
  return f3;
}
inline float3 make_float3(const float &s) { return make_float3(s, s, s); }
inline float3 make_float3(const float4 &a) {
  return make_float3(a.x, a.y, a.z);
}

inline float4 make_float4(const float &a, const float &b, const float &c,
                          const float &d) {
  float4 f4;
  f4.x = a, f4.y = b, f4.z = c, f4.w = d;
  return f4;
}
inline float4 make_float4(const float &s) { return make_float4(s, s, s, s); }
inline float4 make_float4(const float3 &a) {
  return make_float4(a.x, a.y, a.z, 0.0f);
}
inline float4 make_float4(const float3 &a, const float d) {
  return make_float4(a.x, a.y, a.z, d);
}

const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;
constexpr double FACTORIAL_TABLE[] = {
    1.0,                                    // 0!
    1.0,                                    // 1!
    2.0,                                    // 2!
    6.0,                                    // 3!
    24.0,                                   // 4!
    120.0,                                  // 5!
    720.0,                                  // 6!
    5040.0,                                 // 7!
    40320.0,                                // 8!
    362880.0,                               // 9!
    3628800.0,                              // 10!
    39916800.0,                             // 11!
    479001600.0,                            // 12!
    6227020800.0,                           // 13!
    87178291200.0,                          // 14!
    1307674368000.0,                        // 15!
    20922789888000.0,                       // 16!
    355687428096000.0,                      // 17!
    6402373705728000.0,                     // 18!
    121645100408832000.0,                   // 19!
    2432902008176640000.0,                  // 20!
    51090942171709440000.0,                 // 21!
    1124000727777607680000.0,               // 22!
    25852016738884976640000.0,              // 23!
    620448401733239439360000.0,             // 24!
    15511210043330985984000000.0,           // 25!
    403291461126605635584000000.0,          // 26!
    10888869450418352160768000000.0,        // 27!
    304888344611713860501504000000.0,       // 28!
    8841761993739701954543616000000.0,      // 29!
    265252859812191058636308480000000.0,    // 30!
    8222838654177922817725562880000000.0,   // 31!
    263130836933693530167218012160000000.0, // 32!
    8683317618811886495518194401280000000.0 // 33!
};

// Utility Functions

inline double factorial(unsigned int n) {
  return (n <= 33) ? FACTORIAL_TABLE[n] : infinity;
}

inline float fminf(float a, float b) { return a < b ? a : b; }
inline float fmaxf(float a, float b) { return a > b ? a : b; }
inline float rsqrtf(float x) { return 1.0f / sqrtf(x); }
inline float sqrf(float x) { return x * x; }
inline int sqr(int x) { return x * x; }

inline float3 operator-(const float3 &a) {
  return make_float3(-a.x, -a.y, -a.z);
}
inline float4 operator-(const float4 &a) {
  return make_float4(-a.x, -a.y, -a.z, -a.w);
}

inline float3 operator+(const float3 &a, const float3 &b) {
  return make_float3(a.x + b.x, a.y + b.y, a.z + b.z);
}
inline float4 operator+(const float4 &a, const float4 &b) {
  return make_float4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}

inline void operator+=(float3 &a, const float3 &b) {
  a.x += b.x, a.y += b.y, a.z += b.z;
}
inline void operator+=(float4 &a, const float4 &b) {
  a.x += b.x, a.y += b.y, a.z += b.z, a.w += b.w;
}

inline float3 operator+(const float3 &a, float b) {
  return make_float3(a.x + b, a.y + b, a.z + b);
}
inline float3 operator+(const float3 &a, int b) {
  return make_float3(a.x + (float)b, a.y + (float)b, a.z + (float)b);
}
inline float3 operator+(float b, const float3 &a) {
  return make_float3(a.x + b, a.y + b, a.z + b);
}

inline float4 operator+(const float4 &a, float b) {
  return make_float4(a.x + b, a.y + b, a.z + b, a.w + b);
}
inline float4 operator+(const float4 &a, int b) {
  return make_float4(a.x + (float)b, a.y + (float)b, a.z + (float)b,
                     a.w + (float)b);
}
inline float4 operator+(float b, const float4 &a) {
  return make_float4(a.x + b, a.y + b, a.z + b, a.w + b);
}
inline void operator+=(float4 &a, float b) {
  a.x += b;
  a.y += b;
  a.z += b;
  a.w += b;
}
inline void operator+=(float4 &a, int b) {
  a.x += (float)b;
  a.y += (float)b;
  a.z += (float)b;
  a.w += (float)b;
}

inline float3 operator-(const float3 &a, const float3 &b) {
  return make_float3(a.x - b.x, a.y - b.y, a.z - b.z);
}
inline float3 operator-(const float3 &a, float b) {
  return make_float3(a.x - b, a.y - b, a.z - b);
}
inline float3 operator-(const float3 &a, int b) {
  return make_float3(a.x - (float)b, a.y - (float)b, a.z - (float)b);
}
inline void operator-=(float3 &a, const float3 &b) {
  a.x -= b.x;
  a.y -= b.y;
  a.z -= b.z;
}
inline void operator-=(float3 &a, float b) {
  a.x -= b;
  a.y -= b;
  a.z -= b;
}
inline void operator-=(float3 &a, int b) {
  a.x -= (float)b;
  a.y -= (float)b;
  a.z -= (float)b;
}

inline float4 operator-(const float4 &a, const float4 &b) {
  return make_float4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}
inline float4 operator-(const float4 &a, float b) {
  return make_float4(a.x - b, a.y - b, a.z - b, a.w - b);
}
inline float4 operator-(const float4 &a, int b) {
  return make_float4(a.x - (float)b, a.y - (float)b, a.z - (float)b,
                     a.w - (float)b);
}
inline void operator-=(float4 &a, const float4 &b) {
  a.x -= b.x;
  a.y -= b.y;
  a.z -= b.z;
  a.w -= b.w;
}
inline void operator-=(float4 &a, float b) {
  a.x -= b;
  a.y -= b;
  a.z -= b;
  a.w -= b;
}
inline void operator-=(float4 &a, int b) {
  a.x -= (float)b;
  a.y -= (float)b;
  a.z -= (float)b;
  a.w -= (float)b;
}

inline float3 operator*(const float3 &a, const float3 &b) {
  return make_float3(a.x * b.x, a.y * b.y, a.z * b.z);
}
inline void operator*=(float3 &a, const float3 &b) {
  a.x *= b.x;
  a.y *= b.y;
  a.z *= b.z;
}
inline float3 operator*(const float3 &a, float b) {
  return make_float3(a.x * b, a.y * b, a.z * b);
}
inline float3 operator*(float b, const float3 &a) {
  return make_float3(a.x * b, a.y * b, a.z * b);
}
inline void operator*=(float3 &a, float b) {
  a.x *= b;
  a.y *= b;
  a.z *= b;
}

inline float3 operator/(const float3 &a, const float3 &b) {
  return make_float3(a.x / b.x, a.y / b.y, a.z / b.z);
}
inline void operator/=(float3 &a, const float3 &b) {
  a.x /= b.x;
  a.y /= b.y;
  a.z /= b.z;
}

inline float3 operator/(const float3 &a, float b) {
  return make_float3(a.x / b, a.y / b, a.z / b);
}
inline void operator/=(float3 &a, float b) {
  a.x /= b;
  a.y /= b;
  a.z /= b;
}

inline float3 operator/(float b, const float3 &a) {
  return make_float3(b / a.x, b / a.y, b / a.z);
}
inline void operator/=(float b, const float3 &a) {
  b /= a.x;
  b /= a.y;
  b /= a.z;
}

inline float4 operator*(const float4 &a, const float4 &b) {
  return make_float4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
}
inline void operator*=(float4 &a, const float4 &b) {
  a.x *= b.x;
  a.y *= b.y;
  a.z *= b.z;
  a.w *= b.w;
}
inline float4 operator*(const float4 &a, float b) {
  return make_float4(a.x * b, a.y * b, a.z * b, a.w * b);
}
inline float4 operator*(float b, const float4 &a) {
  return make_float4(a.x * b, a.y * b, a.z * b, a.w * b);
}
inline void operator*=(float4 &a, float b) {
  a.x *= b;
  a.y *= b;
  a.z *= b;
  a.w *= b;
}

inline float4 operator/(const float4 &a, const float4 &b) {
  return make_float4(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w);
}
inline void operator/=(float4 &a, const float4 &b) {
  a.x /= b.x;
  a.y /= b.y;
  a.z /= b.z;
  a.w /= b.w;
}

inline float4 operator/(const float4 &a, float b) {
  return make_float4(a.x / b, a.y / b, a.z / b, a.w / b);
}
inline void operator/=(float4 &a, float b) {
  a.x /= b;
  a.y /= b;
  a.z /= b;
  a.w /= b;
}

inline float4 operator/(float b, const float4 &a) {
  return make_float4(b / a.x, b / a.y, b / a.z, b / a.w);
}
inline void operator/=(float b, const float4 &a) {
  b /= a.x;
  b /= a.y;
  b /= a.z;
  b /= a.w;
}

inline float3 cross(const float3 &a, const float3 &b) {
  return make_float3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z,
                     a.x * b.y - a.y * b.x);
}

inline float dot(const float3 &a, const float3 &b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}
inline float dot(const float4 &a, const float4 &b) {
  return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

inline float length(const float3 &a) { return sqrtf(dot(a, a)); }

inline float3 normalize(const float3 &a) {
  float invLength = rsqrtf(dot(a, a));
  return a * invLength;
}

inline float4 normalize(const float4 &a) {
  float invLength = rsqrtf(dot(a, a));
  return a * invLength;
}

inline float3 fminf(const float3 &a, const float3 &b) {
  return make_float3(fminf(a.x, b.x), fminf(a.y, b.y), fminf(a.z, b.z));
}
inline float4 fminf(const float4 &a, const float4 &b) {
  return make_float4(fminf(a.x, b.x), fminf(a.y, b.y), fminf(a.z, b.z),
                     fminf(a.w, b.w));
}

inline float3 fmaxf(const float3 &a, const float3 &b) {
  return make_float3(fmaxf(a.x, b.x), fmaxf(a.y, b.y), fmaxf(a.z, b.z));
}
inline float4 fmaxf(const float4 &a, const float4 &b) {
  return make_float4(fmaxf(a.x, b.x), fmaxf(a.y, b.y), fmaxf(a.z, b.z),
                     fmaxf(a.w, b.w));
}

inline float clamp(float f, float a, float b) { return fmaxf(a, fminf(f, b)); }
inline int clamp(int f, int a, int b) { return __max(a, __min(f, b)); }
inline int clamp(uint f, uint a, uint b) { return __max(a, __min(f, b)); }

inline float radians(const float &deg) { return (3.14159 / 180) * deg; }

namespace srgb {
constexpr float A = 0.055f;
constexpr float PHI = 12.92f;
constexpr float GAMMA = 2.4f;
constexpr float K0 = 0.04045f;
constexpr float K1 = 0.0031308f;
constexpr float INV_GAMMA = 1.0f / GAMMA;

inline float to_linear(float s) {
  return (s <= K0) ? (s / PHI) : std::pow((s + A) / (1.0f + A), GAMMA);
}

inline float3 to_linear(const float3 &c) {
  return {srgb::to_linear(c.x), srgb::to_linear(c.y), srgb::to_linear(c.z)};
}

inline float from_linear(float l) {
  return (l <= K1) ? (PHI * l) : (1.0f + A) * std::pow(l, INV_GAMMA) - A;
}

} // namespace srgb

#endif // HTBBVH_MATH_H
