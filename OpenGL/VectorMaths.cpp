
#include "VectorMaths.hpp"

using namespace scos;

float VectorMaths::dot(float * a, float * b) {
  return a[X]*b[X] + a[Y]*b[Y] + a[Z]*b[Z];
};

float VectorMaths::dot(float ax, float ay, float az, float bx, float by, float bz) {
  return ax*bx + ay*by + az*bz;
};

float * VectorMaths::cross(float * u, float * v) {
  
  float * cp = (float*)malloc(sizeof(float) * 3);
  
  cp[X] = u[Y]*v[Z] - u[Z]*v[Y];
  cp[Y] = u[Z]*v[X] - u[X]*v[Z];
  cp[Z] = u[X]*v[Y] - u[Y]*v[X];
  
  return cp;
};

float * VectorMaths::getNormalisedVector(float * v) {
  
  float recip_len = invsqrt(sqr(v[X]) + sqr(v[Y]) + sqr(v[Z]));
  
  float * new_v = (float*)malloc(sizeof(float) * 3);
  new_v[X] = v[X] * recip_len;
  new_v[Y] = v[Y] * recip_len;
  new_v[Z] = v[Z] * recip_len;

  return new_v;
};

void VectorMaths::normalise(float * v) {
  
  float recip_len = invsqrt(sqr(v[X]) + sqr(v[Y]) + sqr(v[Z]));
  
  v[X] *= recip_len;
  v[Y] *= recip_len;
  v[Z] *= recip_len;
  
};


float VectorMaths::sqr(float n) {
  return n*n;
};


// This function is not my own work.
// It has been copied from the Quake3 invsqrt function
//   (which at the time of writing this is Open Source).
float VectorMaths::invsqrt(float x) {
  
  float xhalf = 0.5f*x;
  int i = *(int*)&x;
  i = 0x5f3759df - (i>>1);
  x = *(float*)&i;
  x = x*(1.5f - xhalf*x*x);
  return x;
};


float * VectorMaths::plus(float * a, float * b) {
  
  float * new_v = (float*)malloc(sizeof(float) * 3);
  
  new_v[X] = a[X] + b[X];
  new_v[Y] = a[Y] + b[Y];
  new_v[Z] = a[Z] + b[Z];
  
  return new_v;
};

float * VectorMaths::minus(float * a, float * b) {
  
  float * new_v = (float*)malloc(sizeof(float) * 3);
  
  new_v[X] = a[X] - b[X];
  new_v[Y] = a[Y] - b[Y];
  new_v[Z] = a[Z] - b[Z];
  
  return new_v;
  
};

float * VectorMaths::copy(float * v) {
  
  float * new_v = (float*)malloc(sizeof(float) * 3);
  
  new_v[0] = v[0];
  new_v[1] = v[1];
  new_v[2] = v[2];
  
  return new_v;

};

float * VectorMaths::dir2spherical(float * dir) {
  
  float azimuth_rads = atan2(dir[2], dir[0]);
  float xz_plane_radius = sqrt(sqr(dir[0]) + sqr(dir[2]));
  
  float inclination_rads = atan2(dir[1], xz_plane_radius);
  
  float * new_v = (float*)malloc(sizeof(float) * 3);
  new_v[0] = 1.0f;
  new_v[1] = inclination_rads;
  new_v[2] = azimuth_rads;

  return new_v;
};

float * VectorMaths::spherical2dir(float * sph) {
  
  float * new_v = (float*)malloc(sizeof(float) * 3);

  new_v[0] = 1.0f;
  new_v[2] = tan(sph[2]);
  float xz_radius = 1.0 / cos(sph[2]);
  
  new_v[1] = xz_radius * tan(sph[1]);
  
  normalise(new_v);
  
  return new_v;
};



