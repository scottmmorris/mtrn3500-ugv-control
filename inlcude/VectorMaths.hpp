
#ifndef SCOS_VECTOR_MATHS
#define SCOS_VECTOR_MATHS

#include <cstdlib>
#include <cmath>

namespace scos {

  class VectorMaths {
    
  private:
    static const int X = 0;
    static const int Y = 1;
    static const int Z = 2;

  public:
    static inline float sqr(float n);

    static float dot(float * a, float * b);
    static float dot(float ax, float ay, float az, float bx, float by, float bz);
    static float * cross(float * a, float * b);
    
    static float * getNormalisedVector(float * v); // gets a seperate vector
    static void normalise(float * v); // normalises the given vector

    static float invsqrt(float x);    

    static float * plus(float * a, float * b);
    static float * minus(float * a, float * b);

    static float * copy(float * v);

    // dir is a unit directional vector
    // spherical is (r,theta,phi), where:
    //   radius = 1.0
    //   theta = angle of inclination in radians
    //   phi = angle of azimuth from x axis in radians
    static float * dir2spherical(float * dir);
    static float * spherical2dir(float * sph);
    
  };

};

#endif // for SCOS_VECTOR_MATHS
