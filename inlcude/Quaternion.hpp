
#ifndef SCOS_QUATERNION_H
#define SCOS_QUATERNION_H

#include <cmath>
#include <iostream>

namespace scos {
  class Quaternion {
    
  private:

    
  public:
    float x,y,z,w;

    Quaternion();
    Quaternion(const Quaternion & other);
    Quaternion(float x_, float y_, float z_, float w_);
    ~Quaternion();

    float length();
    void normalise();
    Quaternion getConjugate();
    
    Quaternion operator*(Quaternion & b);
    
    void print();
  };
};

#endif // for SCOS_QUATERNION_H
