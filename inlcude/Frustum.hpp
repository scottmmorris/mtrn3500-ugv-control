
#ifndef SCOS_FRUSTUM_H
#define SCOS_FRUSTUM_H

#include <iostream>
#include <cmath>
#include <cstdlib>
#include "VectorMaths.hpp"

namespace scos {

  class Frustum {
    
  private:

	  static const int X = 0;
		static const int Y = 1;
		static const int Z = 2;

		float hfov;    // horizontal field of view (in radians)
		float vfov;    // vertical field of view in radians
		float aspect;  // aspect ratio (horiz/vert)

		// windows.h makes it annoying if you want to use the words 'near' or 'far'... and I do
#ifdef near
#undef near
#endif
#ifdef far
#undef far
#endif

		float near;    // distance to near clip plane
		float far;     // distance to far clip plane

    float m_pos[3]; // a copy of the position of the focal point of the frustum
    float m_dir[3]; // a copy of the direction of the axis of symmetry of the frustum
    float m_up[3];  // a copy of the up vector

    float far_tan_half_vfov;
    float near_tan_half_vfov;
    float far_tan_half_hfov;
    float near_tan_half_hfov;

    float * r_abc, r_d; // right plane
    float * u_abc, u_d; // up plane
    float * l_abc, l_d; // left plane
    float * d_abc, d_d; // down plane
    float * n_abc, n_d; // near plane
    float * f_abc, f_d; // far plane
    
  public:
    Frustum(float vfov_, float aspect_, float near_, float far_);
    ~Frustum();
    
    void update(float * pos, float * dir, float * up);
    
    void renderFrustumPlanes(); // used for debugging (renders based on frustum vertices)
    void renderFrustumPlanes_v2(); // used for debugging (renders based on stored values for a,b,c,d for each plane)

    int pointInFrustum(float * p);
    int sphereInFrustum(float * c, float rad); // c -> center, rad -> radius
    
  };
  
};

#endif // for SCOS_FRUSTUM_H
