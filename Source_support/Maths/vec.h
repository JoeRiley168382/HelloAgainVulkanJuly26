#ifndef _MATHS_VEC_
#define _MATHS_VEC_


namespace maths {

struct vec3f {
    float x = 0.0f, y = 0.0f, z = 0.0f;
    float& operator[](int aIdx) { return (&x)[aIdx]; }
    float operator[](int aIdx) const { return (&x)[aIdx]; }

    vec3f operator+(vec3f const& aRHS) const { return { x+aRHS.x, y+aRHS.y, z+aRHS.z}; }
    vec3f operator-(vec3f const& aRHS) const { return { x-aRHS.x, y-aRHS.y, z-aRHS.z}; }
    vec3f operator*(float aScalar) const { return { x*aScalar, y*aScalar, z*aScalar}; }
    //DOT PRODUCT
    float operator|(vec3f const& aRHS) const { return x*aRHS.x + y*aRHS.y + z*aRHS.z; }
    //Future, cross product ^ operator
};

struct vec4f {
    float x = 0.0f, y = 0.0f, z = 0.0f, w = 0.0f;
    float& operator[](int aIdx) { return (&x)[aIdx]; }
    float operator[](int aIdx) const { return (&x)[aIdx]; }

    vec4f operator+(vec4f const& aRHS) const { return { x+aRHS.x, y+aRHS.y, z+aRHS.z, w+aRHS.w}; }
    vec4f operator-(vec4f const& aRHS) const { return { x-aRHS.x, y-aRHS.y, z-aRHS.z, w-aRHS.w}; }
    vec4f operator*(float aScalar) const { return { x*aScalar, y*aScalar, z*aScalar, w*aScalar}; }
    //DOT PRODUCT
    float operator|(vec4f const& aRHS) const { return x*aRHS.x + y*aRHS.y + z*aRHS.z + w*aRHS.w; }
};

}



#endif