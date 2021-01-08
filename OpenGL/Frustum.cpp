#ifdef __APPLE__
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
	#include <GLUT/glut.h>
#elif defined(WIN32)
	#include <Windows.h>
	#include <GL/gl.h>
	#include <GL/glu.h>
	#include <GL/glut.h>
#else
	#include <GL/gl.h>
	#include <GL/glu.h>
	#include <GL/glut.h>
#endif

#include "Frustum.hpp"

using namespace scos;

Frustum::Frustum(float vfov_, float aspect_, float near_, float far_) {
  hfov = vfov_ * aspect_;
  vfov = vfov_;
  aspect = aspect_;
  near = near_;
  far = far_;
  
  float tan_half_vfov = tan(vfov / 2.0f);
  float tan_half_hfov = tan(hfov / 2.0f);

  far_tan_half_vfov = far * tan_half_vfov;
  near_tan_half_vfov = near * tan_half_vfov;
  
  far_tan_half_hfov = far * tan_half_hfov;
  near_tan_half_hfov = near * tan_half_hfov;

  r_abc = (float*)malloc(sizeof(float) * 3);
  u_abc = (float*)malloc(sizeof(float) * 3);
  l_abc = (float*)malloc(sizeof(float) * 3);
  d_abc = (float*)malloc(sizeof(float) * 3);
  n_abc = (float*)malloc(sizeof(float) * 3);
  f_abc = (float*)malloc(sizeof(float) * 3);
  
};

Frustum::~Frustum() {

  free(r_abc);
  free(u_abc);
  free(l_abc);
  free(d_abc);
  free(n_abc);
  free(f_abc);

};

void Frustum::update(float * pos, float * dir, float * up) {

  m_pos[0] = pos[0]; m_pos[1] = pos[1]; m_pos[2] = pos[2];
  m_dir[0] = dir[0]; m_dir[1] = dir[1]; m_dir[2] = dir[2];
  m_up[0] = up[0]; m_up[1] = up[1]; m_up[2] = up[2];
  
  float * right_dir = VectorMaths::cross(dir, up);
  VectorMaths::normalise(right_dir);

  float * up_dir = VectorMaths::cross(right_dir, dir);
  VectorMaths::normalise(up_dir);
  

  float far_right_dir_tan_half_hfov[3] = 
    {right_dir[X] * far_tan_half_hfov,
     right_dir[Y] * far_tan_half_hfov,
     right_dir[Z] * far_tan_half_hfov};

  float far_up_dir_tan_half_vfov[3] = 
    {up_dir[X] * far_tan_half_vfov,
     up_dir[Y] * far_tan_half_vfov,
     up_dir[Z] * far_tan_half_vfov};
  
  float near_right_dir_tan_half_hfov[3] =
    {right_dir[X] * near_tan_half_hfov,
     right_dir[Y] * near_tan_half_hfov,
     right_dir[Z] * near_tan_half_hfov};
  
  float near_up_dir_tan_half_vfov[3] =
    {up_dir[X] * near_tan_half_vfov,
     up_dir[Y] * near_tan_half_vfov,
     up_dir[Z] * near_tan_half_vfov};


  float far_focal_point[3] = 
    {pos[X] + far * dir[X],
     pos[Y] + far * dir[Y],
     pos[Z] + far * dir[Z]};
  
  float near_focal_point[3] = 
    {pos[X] + near * dir[X],
     pos[Y] + near * dir[Y],
     pos[Z] + near * dir[Z]};
  
  // near plane
  n_abc[0] = dir[0];
  n_abc[1] = dir[1];
  n_abc[2] = dir[2];
  n_d = -n_abc[0] * near_focal_point[X] - n_abc[1] * near_focal_point[Y] - n_abc[2] * near_focal_point[2];
  
  // far plane
  f_abc[0] = -dir[0];
  f_abc[1] = -dir[1];
  f_abc[2] = -dir[2];
  f_d = -f_abc[0] * far_focal_point[X] - f_abc[1] * far_focal_point[Y] - f_abc[2] * far_focal_point[2];
  
  // right plane
  r_abc[X] = near_focal_point[X] + near_right_dir_tan_half_hfov[X] - pos[X];
  r_abc[Y] = near_focal_point[Y] + near_right_dir_tan_half_hfov[Y] - pos[Y];
  r_abc[Z] = near_focal_point[Z] + near_right_dir_tan_half_hfov[Z] - pos[Z];
  VectorMaths::normalise(r_abc);
  r_abc = VectorMaths::cross(up_dir, r_abc);
  r_d = -r_abc[X] * pos[X] - r_abc[Y] * pos[Y] - r_abc[Z] * pos[Z];
  
  // left plane
  l_abc[X] = near_focal_point[X] - near_right_dir_tan_half_hfov[X] - pos[X];
  l_abc[Y] = near_focal_point[Y] - near_right_dir_tan_half_hfov[Y] - pos[Y];
  l_abc[Z] = near_focal_point[Z] - near_right_dir_tan_half_hfov[Z] - pos[Z];
  VectorMaths::normalise(l_abc);
  l_abc = VectorMaths::cross(l_abc, up_dir);
  l_d = -l_abc[X] + pos[X] - l_abc[Y] * pos[Y] - l_abc[Z] * pos[Z];
  
  // up plane
  u_abc[X] = near_focal_point[X] + near_up_dir_tan_half_vfov[X] - pos[X];
  u_abc[Y] = near_focal_point[Y] + near_up_dir_tan_half_vfov[Y] - pos[Y];
  u_abc[Z] = near_focal_point[Z] + near_up_dir_tan_half_vfov[Z] - pos[Z];
  VectorMaths::normalise(u_abc);
  u_abc = VectorMaths::cross(u_abc, right_dir);
  u_d = -u_abc[0] * pos[0] - u_abc[1] * pos[1] - u_abc[2] * pos[2];
  
  // down plane
  d_abc[X] = near_focal_point[X] - near_up_dir_tan_half_vfov[X] - pos[X];
  d_abc[Y] = near_focal_point[Y] - near_up_dir_tan_half_vfov[Y] - pos[Y];
  d_abc[Z] = near_focal_point[Z] - near_up_dir_tan_half_vfov[Z] - pos[Z];
  VectorMaths::normalise(d_abc);
  d_abc = VectorMaths::cross(right_dir, d_abc);
  d_d = -d_abc[0] * pos[X] - d_abc[1] * pos[Y] - d_abc[2] * pos[Z];   
  
  free(right_dir);
  free(up_dir);
  
};


/**
 * Calculates the frustum vertices and renders based on those vertices.
 *  (See also 'renderFrustumPlanes_v2()' function).
 */
void Frustum::renderFrustumPlanes() {
  
  float * right_dir = VectorMaths::cross(m_dir, m_up);
  VectorMaths::normalise(right_dir);

  float * up_dir = VectorMaths::cross(right_dir, m_dir);
  VectorMaths::normalise(up_dir);
  

  float far_right_dir_tan_half_hfov[3] = 
    {right_dir[X] * far_tan_half_hfov,
     right_dir[Y] * far_tan_half_hfov,
     right_dir[Z] * far_tan_half_hfov};

  float far_up_dir_tan_half_vfov[3] = 
    {up_dir[X] * far_tan_half_vfov,
     up_dir[Y] * far_tan_half_vfov,
     up_dir[Z] * far_tan_half_vfov};
  
  float near_right_dir_tan_half_hfov[3] =
    {right_dir[X] * near_tan_half_hfov,
     right_dir[Y] * near_tan_half_hfov,
     right_dir[Z] * near_tan_half_hfov};
  
  float near_up_dir_tan_half_vfov[3] =
    {up_dir[X] * near_tan_half_vfov,
     up_dir[Y] * near_tan_half_vfov,
     up_dir[Z] * near_tan_half_vfov};


  float far_focal_point[3] = 
    {m_pos[X] + far * m_dir[X],
     m_pos[Y] + far * m_dir[Y],
     m_pos[Z] + far * m_dir[Z]};
  
  float near_focal_point[3] = 
    {m_pos[X] + near * m_dir[X],
     m_pos[Y] + near * m_dir[Y],
     m_pos[Z] + near * m_dir[Z]};
  

  // the 8 vertices of the frustum...
  // ftr -> far-top-right
  // nbl -> near-bottom-left  
  float ftr[3] = 
    {far_focal_point[X] + far_up_dir_tan_half_vfov[X] + far_right_dir_tan_half_hfov[X],
     far_focal_point[Y] + far_up_dir_tan_half_vfov[Y] + far_right_dir_tan_half_hfov[Y],
     far_focal_point[Z] + far_up_dir_tan_half_vfov[Z] + far_right_dir_tan_half_hfov[Z]};
  
  float ftl[3] = 
    {far_focal_point[X] + far_up_dir_tan_half_vfov[X] - far_right_dir_tan_half_hfov[X],
     far_focal_point[Y] + far_up_dir_tan_half_vfov[Y] - far_right_dir_tan_half_hfov[Y],
     far_focal_point[Z] + far_up_dir_tan_half_vfov[Z] - far_right_dir_tan_half_hfov[Z]};
     
  float fbl[3] = 
    {far_focal_point[X] - far_up_dir_tan_half_vfov[X] - far_right_dir_tan_half_hfov[X],
     far_focal_point[Y] - far_up_dir_tan_half_vfov[Y] - far_right_dir_tan_half_hfov[Y],
     far_focal_point[Z] - far_up_dir_tan_half_vfov[Z] - far_right_dir_tan_half_hfov[Z]};
     
  float fbr[3] = 
    {far_focal_point[X] - far_up_dir_tan_half_vfov[X] + far_right_dir_tan_half_hfov[X],
     far_focal_point[Y] - far_up_dir_tan_half_vfov[Y] + far_right_dir_tan_half_hfov[Y],
     far_focal_point[Z] - far_up_dir_tan_half_vfov[Z] + far_right_dir_tan_half_hfov[Z]};
  
  
  float ntr[3] = 
    {near_focal_point[X] + near_up_dir_tan_half_vfov[X] + near_right_dir_tan_half_hfov[X],
     near_focal_point[Y] + near_up_dir_tan_half_vfov[Y] + near_right_dir_tan_half_hfov[Y],
     near_focal_point[Z] + near_up_dir_tan_half_vfov[Z] + near_right_dir_tan_half_hfov[Z]};
     
  float ntl[3] = 
    {near_focal_point[X] + near_up_dir_tan_half_vfov[X] - near_right_dir_tan_half_hfov[X],
     near_focal_point[Y] + near_up_dir_tan_half_vfov[Y] - near_right_dir_tan_half_hfov[Y],
     near_focal_point[Z] + near_up_dir_tan_half_vfov[Z] - near_right_dir_tan_half_hfov[Z]};

  float nbl[3] = 
    {near_focal_point[X] - near_up_dir_tan_half_vfov[X] - near_right_dir_tan_half_hfov[X],
     near_focal_point[Y] - near_up_dir_tan_half_vfov[Y] - near_right_dir_tan_half_hfov[Y],
     near_focal_point[Z] - near_up_dir_tan_half_vfov[Z] - near_right_dir_tan_half_hfov[Z]};

  float nbr[3] = 
    {near_focal_point[X] - near_up_dir_tan_half_vfov[X] + near_right_dir_tan_half_hfov[X],
     near_focal_point[Y] - near_up_dir_tan_half_vfov[Y] + near_right_dir_tan_half_hfov[Y],
     near_focal_point[Z] - near_up_dir_tan_half_vfov[Z] + near_right_dir_tan_half_hfov[Z]};
  
  glColor4f(1,1,0,0.5);
  
  glBegin(GL_QUADS);
  
  // right plane
  glVertex3fv(ftr);
  glVertex3fv(ntr);
  glVertex3fv(nbr);
  glVertex3fv(fbr);
  
  // left plane
  glVertex3fv(ftl);
  glVertex3fv(ntl);
  glVertex3fv(nbl);
  glVertex3fv(fbl);  

  // up plane
  glVertex3fv(ftr);
  glVertex3fv(ntr);
  glVertex3fv(ntl);
  glVertex3fv(ftl);
  
  // down plane
  glVertex3fv(fbr);
  glVertex3fv(nbr);
  glVertex3fv(nbl);
  glVertex3fv(fbl);  
  
  glEnd();  
  
};


/**
 * Renders the planes based on stored a,b,c,d values for each plane.
 *  (See also, v1 of this function, which renders the planes
 *   based on other values).
 */
void Frustum::renderFrustumPlanes_v2() {
  
  
  
};

int Frustum::pointInFrustum(float * p) {
  
  if (n_abc[0] * p[X] + n_abc[1] * p[Y] + n_abc[2] * p[Z] + n_d < 0) return 0;
  if (f_abc[0] * p[X] + f_abc[1] * p[Y] + f_abc[2] * p[Z] + f_d < 0) return 0;
  if (r_abc[0] * p[X] + r_abc[1] + p[Y] + r_abc[2] * p[Z] + r_d < 0) return 0;
  if (l_abc[0] * p[X] + l_abc[1] + p[Y] + l_abc[2] * p[Z] + l_d < 0) return 0;
  if (d_abc[0] * p[X] + d_abc[1] + p[Y] + d_abc[2] * p[Z] + d_d < 0) return 0;
  if (u_abc[0] * p[X] + u_abc[1] + p[Y] + u_abc[2] * p[Z] + u_d < 0) return 0;
  
  return 1;
  
};


int Frustum::sphereInFrustum(float * c, float rad) {
  
  if (n_abc[0] * c[X] + n_abc[1] * c[1] + n_abc[2] * c[Z] + n_d < -rad) return 0;
  if (f_abc[0] * c[X] + f_abc[1] * c[1] + f_abc[2] * c[Z] + f_d < -rad) return 0;
  if (r_abc[0] * c[X] + r_abc[1] * c[1] + r_abc[2] * c[Z] + r_d < -rad) return 0;
  if (l_abc[0] * c[X] + l_abc[1] * c[1] + l_abc[2] * c[Z] + l_d < -rad) return 0;
  if (d_abc[0] * c[X] + d_abc[1] * c[1] + d_abc[2] * c[Z] + d_d < -rad) return 0;
  if (u_abc[0] * c[X] + u_abc[1] * c[1] + u_abc[2] * c[Z] + u_d < -rad) return 0;
  
  return 1;

};

