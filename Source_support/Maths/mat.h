#ifndef _MATHS_MAT_
#define _MATHS_MAT_

#include "vec.h"

namespace maths {

// Column-major storage: col[c] is the c-th column, matching GLSL's
// default mat4 layout so this uploads to a UBO/push-constant with no transpose.
struct mat4f {
    vec4f col[4]{};

    static mat4f Identity() {
        mat4f m;
        m.col[0] = {1,0,0,0};
        m.col[1] = {0,1,0,0};
        m.col[2] = {0,0,1,0};
        m.col[3] = {0,0,0,1};
        return m;
    }

    vec4f& operator[](int aCol) { return col[aCol]; }
    vec4f const& operator[](int aCol) const { return col[aCol]; }

    vec4f operator*(vec4f const& aRHS) const {
        return col[0]*aRHS.x + col[1]*aRHS.y + col[2]*aRHS.z + col[3]*aRHS.w;
    }

    mat4f operator*(mat4f const& aRHS) const {
        mat4f result;
        for(int c = 0; c < 4; c++)
            result.col[c] = (*this) * aRHS.col[c];
        return result;
    }
};

}

#endif