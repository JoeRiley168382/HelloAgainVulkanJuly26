#ifndef _MATHS_TRANSFORM_
#define _MATHS_TRANSFORM_

#include "mat.h"
#include "vec.h"

#include <cmath>

namespace maths {

inline mat4f Translate(vec3f const& aOffset) {
    mat4f m = mat4f::Identity();
    m.col[3] = { aOffset.x, aOffset.y, aOffset.z, 1.0f };
    return m;
}

inline mat4f Scale(vec3f const& aScale) {
    mat4f m;
    m.col[0] = { aScale.x, 0.0f, 0.0f, 0.0f };
    m.col[1] = { 0.0f, aScale.y, 0.0f, 0.0f };
    m.col[2] = { 0.0f, 0.0f, aScale.z, 0.0f };
    m.col[3] = { 0.0f, 0.0f, 0.0f, 1.0f };
    return m;
}

// Right-handed rotation, angle in radians, positive = counter-clockwise
// looking from the positive axis toward the origin.
inline mat4f RotateRoll(float aRadians) {
    float const c = std::cos(aRadians);
    float const s = std::sin(aRadians);
    mat4f m;
    m.col[0] = { 1.0f, 0.0f, 0.0f, 0.0f };
    m.col[1] = { 0.0f, c,    s,    0.0f };
    m.col[2] = { 0.0f, -s,   c,    0.0f };
    m.col[3] = { 0.0f, 0.0f, 0.0f, 1.0f };
    return m;
}

inline mat4f RotateYaw(float aRadians) {
    float const c = std::cos(aRadians);
    float const s = std::sin(aRadians);
    mat4f m;
    m.col[0] = { c,    0.0f, -s,   0.0f };
    m.col[1] = { 0.0f, 1.0f, 0.0f, 0.0f };
    m.col[2] = { s,    0.0f, c,    0.0f };
    m.col[3] = { 0.0f, 0.0f, 0.0f, 1.0f };
    return m;
}

inline mat4f RotateTilt(float aRadians) {
    float const c = std::cos(aRadians);
    float const s = std::sin(aRadians);
    mat4f m;
    m.col[0] = { c,    s,    0.0f, 0.0f };
    m.col[1] = { -s,   c,    0.0f, 0.0f };
    m.col[2] = { 0.0f, 0.0f, 1.0f, 0.0f };
    m.col[3] = { 0.0f, 0.0f, 0.0f, 1.0f };
    return m;
}

// Right-handed, depth range [0,1] (Vulkan), with the Y term negated to
// account for Vulkan's Y-down NDC (GLM's equivalent needs the same flip
// when its output is fed to Vulkan instead of OpenGL).
inline mat4f Perspective(float aFovYRadians, float aAspect, float aNear, float aFar) {
    float const f = 1.0f / std::tan(aFovYRadians * 0.5f);
    mat4f m{};
    m.col[0] = { f / aAspect, 0.0f, 0.0f, 0.0f };
    m.col[1] = { 0.0f, -f, 0.0f, 0.0f };
    m.col[2] = { 0.0f, 0.0f, aFar / (aNear - aFar), -1.0f };
    m.col[3] = { 0.0f, 0.0f, (aNear * aFar) / (aNear - aFar), 0.0f };
    return m;
}

}

#endif
