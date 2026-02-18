#ifndef MATH3D_H
#define MATH3D_H

#include <Hx86/stdint.h>

// ============================================================================
// CORE MATH FUNCTIONS
// ============================================================================

static inline float abs(float x) {
    return (x < 0) ? -x : x;
}

static inline float min(float a, float b) {
    return (a < b) ? a : b;
}

static inline float max(float a, float b) {
    return (a > b) ? a : b;
}

// Absolute value (float)
inline float fabsf(float x) {
    return x < 0.0f ? -x : x;
}

// Floor (float)
// Returns the greatest integer <= x, as float
inline float floorf(float x) {
    int i = (int)x;

    // If x is negative and has fractional part, subtract 1
    return (x < 0.0f && x != (float)i) ? (float)(i - 1) : (float)i;
}

static inline float clamp(float value, float minVal, float maxVal) {
    return max(minVal, min(maxVal, value));
}

// Fast inverse square root (Quake III algorithm)
static inline float fast_inv_sqrt(float number) {
    long i;
    float x2, y;
    const float threehalfs = 1.5F;

    x2 = number * 0.5F;
    y = number;
    i = *(long*)&y;
    i = 0x5f3759df - (i >> 1);
    y = *(float*)&i;
    y = y * (threehalfs - (x2 * y * y));  // Newton iteration
    // y  = y * ( threehalfs - ( x2 * y * y ) ); // 2nd iteration for more accuracy

    return y;  // This IS 1/sqrt(number)
}

// Regular square root
static inline float sqrt(float number) {
    if (number <= 0.0f) return 0.0f;
    return 1.0f / fast_inv_sqrt(number);
}

// Trigonometric functions using x87 FPU
static inline float sin(float x) {
    float res;
    asm volatile("fsin" : "=t"(res) : "0"(x));
    return res;
}

static inline float cos(float x) {
    float res;
    asm volatile("fcos" : "=t"(res) : "0"(x));
    return res;
}

static inline float tan(float x) {
    float s, c;
    asm volatile("fsincos" : "=t"(c), "=u"(s) : "0"(x));
    return s / c;
}

// Arc Tangent 2 (returns angle in radians between -PI and PI)
static inline float atan2(float y, float x) {
    float res;
    // fpatan computes arctan(st(1)/st(0)) -> arctan(y/x)
    asm volatile(
        "fld %1\n\t"  // Load Y
        "fld %2\n\t"  // Load X
        "fpatan"      // Calculate atan2(y, x)
        : "=t"(res)
        : "m"(y), "m"(x));
    return res;
}

// Arc Sine
static inline float asin(float x) {
    // asin(x) = atan2(x, sqrt(1 - x*x))
    if (x > 1.0f) x = 1.0f;
    if (x < -1.0f) x = -1.0f;
    return atan2(x, sqrt(1.0f - x * x));
}

// Ceiling (float)
// Returns the smallest integer >= x, as float
static inline float ceilf(float x) {
    int i = (int)x;
    // If x is positive and has a fractional part, we add 1
    // (int conversion truncates toward zero)
    return (x > (float)i) ? (float)(i + 1) : (float)i;
}

// Useful constants
#define PI 3.14159265358979323846f
#define TWO_PI 6.28318530717958647692f
#define HALF_PI 1.57079632679489661923f
#define DEG_TO_RAD 0.01745329251994329576f
#define RAD_TO_DEG 57.2957795130823208767f

// ============================================================================
// VEC3 - ENHANCED
// ============================================================================
struct Vec3 {
    float x, y, z;

    Vec3() : x(0), y(0), z(0) {}
    Vec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

    // [Keep your existing operators: +, -, *, /]
    Vec3 operator+(const Vec3& r) const {
        return Vec3(x + r.x, y + r.y, z + r.z);
    }
    Vec3 operator-(const Vec3& r) const {
        return Vec3(x - r.x, y - r.y, z - r.z);
    }
    Vec3 operator*(float r) const {
        return Vec3(x * r, y * r, z * r);
    }
    Vec3 operator/(float r) const {
        float inv = 1.0f / r;
        return Vec3(x * inv, y * inv, z * inv);
    }

    float Dot(const Vec3& r) const {
        return x * r.x + y * r.y + z * r.z;
    }

    Vec3 Cross(const Vec3& r) const {
        return Vec3(y * r.z - z * r.y, z * r.x - x * r.z, x * r.y - y * r.x);
    }

    float LengthSquared() const {
        return x * x + y * y + z * z;
    }

    void Normalize() {
        float lenSq = LengthSquared();
        if (lenSq > 0.0f) {
            float invLen = fast_inv_sqrt(lenSq);
            x *= invLen;
            y *= invLen;
            z *= invLen;
        }
    }

    Vec3 Normalized() const {
        Vec3 res = *this;
        res.Normalize();
        return res;
    }

    // NEW: Critical for Clipping
    static Vec3 Lerp(const Vec3& a, const Vec3& b, float t) {
        return Vec3(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t);
    }
};

// ============================================================================
// VEC2 - ENHANCED
// ============================================================================
struct Vec2 {
    float x, y;
    Vec2() : x(0), y(0) {}
    Vec2(float _x, float _y) : x(_x), y(_y) {}

    // NEW: Critical for Clipping
    static Vec2 Lerp(const Vec2& a, const Vec2& b, float t) {
        return Vec2(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t);
    }
};

// ============================================================================
// MAT4 - 4x4 MATRIX
// ============================================================================

struct Mat4 {
    float m[4][4];

    // Constructor
    Mat4() {
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++) m[i][j] = 0.0f;
    }

    // Identity matrix
    static Mat4 Identity() {
        Mat4 res;
        res.m[0][0] = 1.0f;
        res.m[1][1] = 1.0f;
        res.m[2][2] = 1.0f;
        res.m[3][3] = 1.0f;
        return res;
    }

    // Translation matrix
    static Mat4 Translation(float x, float y, float z) {
        Mat4 res = Identity();
        res.m[3][0] = x;
        res.m[3][1] = y;
        res.m[3][2] = z;
        return res;
    }

    // Scale matrix
    static Mat4 Scale(float x, float y, float z) {
        Mat4 res = Identity();
        res.m[0][0] = x;
        res.m[1][1] = y;
        res.m[2][2] = z;
        return res;
    }

    // Rotation around X axis
    static Mat4 RotationX(float angle) {
        Mat4 res = Identity();
        float c = cos(angle);
        float s = sin(angle);
        res.m[1][1] = c;
        res.m[1][2] = -s;
        res.m[2][1] = s;
        res.m[2][2] = c;
        return res;
    }

    // Rotation around Y axis
    static Mat4 RotationY(float angle) {
        Mat4 res = Identity();
        float c = cos(angle);
        float s = sin(angle);
        res.m[0][0] = c;
        res.m[0][2] = s;
        res.m[2][0] = -s;
        res.m[2][2] = c;
        return res;
    }

    // Rotation around Z axis
    static Mat4 RotationZ(float angle) {
        Mat4 res = Identity();
        float c = cos(angle);
        float s = sin(angle);
        res.m[0][0] = c;
        res.m[0][1] = -s;
        res.m[1][0] = s;
        res.m[1][1] = c;
        return res;
    }

    // Perspective projection matrix
    static Mat4 Perspective(float fov, float aspect, float near, float far) {
        Mat4 res;
        float tanHalfFov = tan(fov / 2.0f);

        res.m[0][0] = 1.0f / (aspect * tanHalfFov);
        res.m[1][1] = 1.0f / tanHalfFov;
        res.m[2][2] = -(far + near) / (far - near);
        res.m[2][3] = -1.0f;
        res.m[3][2] = -(2.0f * far * near) / (far - near);

        return res;
    }

    // Look-at matrix (camera view matrix)
    static Mat4 LookAt(const Vec3& eye, const Vec3& target, const Vec3& up) {
        Vec3 zAxis = (eye - target).Normalized();
        Vec3 xAxis = up.Cross(zAxis).Normalized();
        Vec3 yAxis = zAxis.Cross(xAxis);

        Mat4 res = Identity();

        res.m[0][0] = xAxis.x;
        res.m[1][0] = xAxis.y;
        res.m[2][0] = xAxis.z;
        res.m[3][0] = -xAxis.Dot(eye);

        res.m[0][1] = yAxis.x;
        res.m[1][1] = yAxis.y;
        res.m[2][1] = yAxis.z;
        res.m[3][1] = -yAxis.Dot(eye);

        res.m[0][2] = zAxis.x;
        res.m[1][2] = zAxis.y;
        res.m[2][2] = zAxis.z;
        res.m[3][2] = -zAxis.Dot(eye);

        return res;
    }

    // Matrix multiplication
    Mat4 operator*(const Mat4& r) const {
        Mat4 res;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                res.m[i][j] = 0.0f;
                for (int k = 0; k < 4; k++) {
                    res.m[i][j] += m[i][k] * r.m[k][j];
                }
            }
        }
        return res;
    }

    // Transform a Vec3 (treating it as a point with w=1)
    Vec3 MultiplyPoint(const Vec3& v) const {
        float x = v.x * m[0][0] + v.y * m[1][0] + v.z * m[2][0] + m[3][0];
        float y = v.x * m[0][1] + v.y * m[1][1] + v.z * m[2][1] + m[3][1];
        float z = v.x * m[0][2] + v.y * m[1][2] + v.z * m[2][2] + m[3][2];
        float w = v.x * m[0][3] + v.y * m[1][3] + v.z * m[2][3] + m[3][3];

        if (w != 0.0f) {
            float invW = 1.0f / w;
            x *= invW;
            y *= invW;
            z *= invW;
        }

        return Vec3(x, y, z);
    }

    // Transform a Vec3 (treating it as a direction with w=0)
    Vec3 MultiplyDirection(const Vec3& v) const {
        float x = v.x * m[0][0] + v.y * m[1][0] + v.z * m[2][0];
        float y = v.x * m[0][1] + v.y * m[1][1] + v.z * m[2][1];
        float z = v.x * m[0][2] + v.y * m[1][2] + v.z * m[2][2];

        return Vec3(x, y, z);
    }

    // Legacy compatibility
    Vec3 Multiply(const Vec3& v) const {
        return MultiplyPoint(v);
    }
};

// ============================================================================
// GEOMETRY STRUCTURES
// ============================================================================

struct Triangle {
    Vec3 p[3];   // Vertex positions
    Vec3 n[3];   // Vertex normals (for smooth shading)
    Vec2 uv[3];  // Texture coordinates

    Triangle() {
        for (int i = 0; i < 3; i++) {
            p[i] = Vec3(0, 0, 0);
            n[i] = Vec3(0, 1, 0);
            uv[i] = Vec2(0, 0);
        }
    }

    // Calculate face normal
    Vec3 GetFaceNormal() const {
        Vec3 edge1 = p[1] - p[0];
        Vec3 edge2 = p[2] - p[0];
        return edge1.Cross(edge2).Normalized();
    }
};

struct Mesh {
    Triangle* tris;
    int triCount;

    Mesh() : tris(nullptr), triCount(0) {}

    ~Mesh() {
        if (tris) {
            delete[] tris;
            tris = nullptr;
        }
    }

    // Calculate bounding box
    void GetBounds(Vec3& minBounds, Vec3& maxBounds) const {
        if (triCount == 0) return;

        minBounds = tris[0].p[0];
        maxBounds = tris[0].p[0];

        for (int t = 0; t < triCount; t++) {
            for (int v = 0; v < 3; v++) {
                Vec3 p = tris[t].p[v];

                if (p.x < minBounds.x) minBounds.x = p.x;
                if (p.y < minBounds.y) minBounds.y = p.y;
                if (p.z < minBounds.z) minBounds.z = p.z;

                if (p.x > maxBounds.x) maxBounds.x = p.x;
                if (p.y > maxBounds.y) maxBounds.y = p.y;
                if (p.z > maxBounds.z) maxBounds.z = p.z;
            }
        }
    }
};

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

// Linear interpolation
static inline float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

// Smooth step interpolation
static inline float smoothstep(float edge0, float edge1, float x) {
    float t = clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

#endif  // MATH3D_H
