
#include "Quaternion.hpp"

using namespace scos;

Quaternion::Quaternion() {

};

Quaternion::Quaternion(const Quaternion & other) {
  this->x = other.x;
  this->y = other.y;
  this->z = other.z;
  this->w = other.w;
};

Quaternion::Quaternion(float x_, float y_, float z_, float w_) {
  x=x_; y=y_; z=z_; w=w_;
};

Quaternion::~Quaternion() {

};

float Quaternion::length() {
  return sqrtf(x*x + y*y + z*z + w*w);
};

void Quaternion::normalise() {
  float len = length();
  x /= len;
  y /= len;
  z /= len;
  w /= len;
};

Quaternion Quaternion::getConjugate() {
  return Quaternion(-x, -y, -z, w);
};

Quaternion Quaternion::operator*(Quaternion & b) {
  float new_x = this->w*b.x + this->x*b.w + this->y*b.z - this->z*b.y;
  float new_y = this->w*b.y - this->x*b.z + this->y*b.w + this->z*b.x;
  float new_z = this->w*b.z + this->x*b.y - this->y*b.x + this->z*b.w;
  float new_w = this->w*b.w - this->x*b.x - this->y*b.y - this->z*b.z;
  return Quaternion(new_x, new_y, new_z, new_w);
};

void Quaternion::print() {
  std::cout << "(" << x << "," << y << "," << z << "," << w << ")" << std::endl;
};
