
#include "Camera.hpp"

#ifdef __APPLE__
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
#elif defined(WIN32)
	#include <Windows.h>
	#include <GL/gl.h>
	#include <GL/glu.h>
#else
	#include <GL/gl.h>
	#include <GL/glu.h>
#endif

using namespace scos;

Camera * Camera::instance = NULL;

Camera::Camera() {

  gradual_transitions = true;
  current_vertical_rotation = 0.0;
  fix_to_point = false;
  
  viewmode = VIEW_FREE;

  viewpos[X] = destpos[X] = 0.0f;
  viewpos[Y] = destpos[Y] = 10.0f;
  viewpos[Z] = destpos[Z] = -30.0f;

  viewdir[X] = destdir[X] = 0.0f;
  viewdir[Y] = destdir[Y] = 0.0f;
  viewdir[Z] = destdir[Z] = 1.0f;

  up[X] = 0.0f;
  up[Y] = 1.0f;
  up[Z] = 0.0f;
  
  shakepos[X] = shakepos[Y] = shakepos[Z] = 0.0;
  
  horizontal_rotation_speed = -0.005;
  vertical_rotation_speed = -0.002;
  movement_speed = 0.5;

  near_clipping_plane = 1.0f;
  far_clipping_plane = 3500.0f;

  followdir[X] = 0.0f;
  followdir[Y] = 0.0f;
  followdir[Z] = 1.0f;

  followdist = 10.0f;

  frustum = NULL;
  manualPursuit = false;
};

Camera::~Camera() {
  
  if (frustum != NULL) {
    delete frustum;
  }
  
};


Camera * Camera::get() {
  
  if (instance == NULL) {
    instance = new Camera();
  }
  
  return instance;

};


void Camera::setWindowDimensions(int wind_width, int wind_height) {
  
  window_width = wind_width;
  window_height = wind_height;
  
  horizontal_half_FOV_dot_product = (30 * 3.1415926535 / 180 * getAspectRatio());

  setViewPort();
  setProjection();

};

float Camera::getAspectRatio() {
  
  return window_width/(float)window_height;
  
};

float Camera::getHorizontalHalfFOVDotProduct() {
  
  return horizontal_half_FOV_dot_product;
  
};

void Camera::setViewPort() {
  glViewport(0, 0, window_width, window_height);
};

void Camera::setProjection() {
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(60, window_width/(float)window_height, near_clipping_plane, far_clipping_plane);
#ifdef VERBOSE
  std::cout << "Camera::setProjection():: window_width=" << window_width << ", window_height=" << window_height << std::endl;
#endif
  
  if (frustum != NULL) {
    delete frustum;
  }
  frustum = new Frustum(60, window_width/(float)window_height, near_clipping_plane, far_clipping_plane);
  
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

};

void Camera::setCameraToDestination() {
  
  normaliseDestDir();

  for (int i = 0; i < 3; ++i) {
    viewpos[i] = destpos[i];
    viewdir[i] = destdir[i];
  }
  
  //printLocation();

};

void Camera::setDestPos(float x_, float y_, float z_) {
  
  destpos[X] = x_;
  destpos[Y] = y_;
  destpos[Z] = z_;
  
};

void Camera::setDestDir(float x_, float y_, float z_) {
  
  destdir[X] = x_;
  destdir[Y] = y_;
  destdir[Z] = z_;
  
};

void Camera::setShakePos(float x_, float y_, float z_) {
  
  shakepos[X] = x_;
  shakepos[Y] = y_;
  shakepos[Z] = z_;
  
};

float * Camera::getViewPos() {
  return viewpos;    // this used to be 'destpos', don't know why
};

float * Camera::getViewDir() {
  return viewdir;    // this used to be 'destdir', don't know why
};

float * Camera::getUpVector() {
  return up;
};

float * Camera::getFollowDir() {
  return followdir;
};

float Camera::getFollowDist() {
  return followdist;
};

void Camera::switchTo2DDrawing() {

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0.0, window_width, window_height, 0.0, -1.0, 1.0);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

};

void Camera::switchTo3DDrawing() {
  
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  
};

void Camera::setLookAt() {
  
#ifdef VERBOSE
  std::cout << "Camera::setLookAt()::(" 
	    << viewpos[X] << "," << viewpos[Y] << "," << viewpos[Z] << ")->("
	    << viewdir[X] << "," << viewdir[Y] << "," << viewdir[Z]
	    << ")" << std::endl;
#endif

  if (sqr(shakepos[X]) + sqr(shakepos[Y]) + sqr(shakepos[Z]) < 0.1) {
    shakepos[X] = shakepos[Y] = shakepos[Z] = 0.0;
  } else {
    shakepos[X] *= -0.9;
    shakepos[Y] *= -0.9;
    shakepos[Z] *= -0.9;
  }
  
  gluLookAt(viewpos[X]+shakepos[X], viewpos[Y]+shakepos[Y], viewpos[Z]+shakepos[Z],
	    viewpos[X]+shakepos[X]+viewdir[X], viewpos[Y]+shakepos[Y]+viewdir[Y], viewpos[Z]+shakepos[Z]+viewdir[Z],
	    up[X], up[Y], up[Z]);

  frustum->update(VectorMaths::plus(viewpos, shakepos), viewdir, up);
  
};

void Camera::setLookAt(int eye) {

  if (eye == 0) {
    this->setLookAt();
    return;
  }
  
  float eye_displacement[3];
  eye_displacement[X] = viewdir[Y]*up[Z] - viewdir[Z]*up[Y];
  eye_displacement[Y] = viewdir[Z]*up[X] - viewdir[X]*up[Z];
  eye_displacement[Z] = viewdir[X]*up[Y] - viewdir[Y]*up[X];
  float eye_disp_length = dist(eye_displacement[X], eye_displacement[Y], eye_displacement[Z]);

  for (int i = 0; i < 3; i++) {
    eye_displacement[i] /= eye_disp_length;
    eye_displacement[i] *= eye_separation * eye;
  }

  if (use_toe_in_method) {
    float eye_rotation_dir[3];
    eye_rotation_dir[X] = viewdir[X] * cos(this->eye_angle_radians * eye) - viewdir[Z] * sin(this->eye_angle_radians * eye);
    eye_rotation_dir[Y] = viewdir[Y];
    eye_rotation_dir[Z] = viewdir[X] * sin(this->eye_angle_radians * eye) + viewdir[Z] * cos(this->eye_angle_radians * eye);
    
    gluLookAt(viewpos[X] + eye_displacement[X],
	      viewpos[Y] + eye_displacement[Y],
	      viewpos[Z] + eye_displacement[Z],
	      viewpos[X] + eye_displacement[X] + eye_rotation_dir[X],
	      viewpos[Y] + eye_displacement[Y] + eye_rotation_dir[Y],
	      viewpos[Z] + eye_displacement[Z] + eye_rotation_dir[Z],
	      up[X],
	      up[Y],
	      up[Z]);

#ifdef VERBOSE
  std::cout << "Camera::setLookAt(" << (eye < 0 ? "left" : "right") << ")::(" 
	    << (viewpos[X] + eye_displacement[X]) << "," 
	    << (viewpos[Y] + eye_displacement[Y]) << ","
            << (viewpos[Z] + eye_displacement[Z]) << ")->("
            << (eye_rotation_dir[X]) << ","
	    << (eye_rotation_dir[Y]) << ","
	    << (eye_rotation_dir[Z]) << ")" << std::endl;

#endif

  } else { // use asymmetric frustum method

    float ratio = window_width / (float) window_height;
    float half_fov_width_radians = this->aperture_degs * 3.14159265 / 180.0;
    float half_width = near_clipping_plane * tan(half_fov_width_radians);
    float ratio_near_to_focal_length = near_clipping_plane / focal_length;
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float left_plane  = -ratio * half_width - eye * 0.5 * eye_separation * ratio_near_to_focal_length;
    float right_plane =  ratio * half_width - eye * 0.5 * eye_separation * ratio_near_to_focal_length;
    
    float top_plane = half_width;
    float bottom_plane = -half_width;
    
    glFrustum(left_plane, right_plane,
	      bottom_plane, top_plane,
	      near_clipping_plane, far_clipping_plane);

    glMatrixMode(GL_MODELVIEW);
    glDrawBuffer(eye < 0 ? GL_BACK_LEFT : GL_BACK_RIGHT);
    glLoadIdentity();
    gluLookAt(viewpos[X] + eye_displacement[X],
	      viewpos[Y] + eye_displacement[Y],
	      viewpos[Z] + eye_displacement[Z],
	      viewpos[X] + eye_displacement[X] + viewdir[X],
	      viewpos[Y] + eye_displacement[Y] + viewdir[Y],
	      viewpos[Z] + eye_displacement[Z] + viewdir[Z],
	      up[X], up[Y], up[Z]);

#ifdef VERBOSE
    std::cout << "Camera::setLookAt(" << (eye < 0 ? "left" : "right") << ")::(" 
	      << (viewpos[X] + eye_displacement[X]) << "," 
	      << (viewpos[Y] + eye_displacement[Y]) << ","
	      << (viewpos[Z] + eye_displacement[Z]) << ")->("
	      << (viewdir[X]) << ","
	      << (viewdir[Y]) << ","
	      << (viewdir[Z]) << ")" << std::endl;
    
#endif

    
  }

  

};

void Camera::setEyeSeparation(float sep) {
  this->eye_separation = sep;
};

float Camera::getEyeSeparation() {
  return this->eye_separation;
};

void Camera::setEyeAngle(float angle) {
  this->eye_angle_radians = angle;
};

float Camera::getEyeAngle() {
  return this->eye_angle_radians;
};

void Camera::updateLocation() {

#if 0
  std::cout << "(" << viewpos[X] << "," << viewpos[Y] << "," << viewpos[Z] << ")->" 
	    << "(" << viewdir[X] << "," << viewdir[Y] << "," << viewdir[Z] << ")" << std::endl;
#endif
  
  if (cameraAtDestination()) return;
  if (cameraNearDestination()) {setCameraToDestination(); return;}

  if (gradual_transitions) {
    float distX = destpos[X] - viewpos[X];
    float distY = destpos[Y] - viewpos[Y];
    float distZ = destpos[Z] - viewpos[Z];
    
    float distToGo = this->dist(distX, distY, distZ);
    
    float ratio = sqrtf(1.0 / distToGo);
    
    viewpos[X] += (distX * ratio);
    viewpos[Y] += (distY * ratio);
    viewpos[Z] += (distZ * ratio);
        
    viewdir[X] += ((destdir[X] - viewdir[X]) * ratio);
    viewdir[Y] += ((destdir[Y] - viewdir[Y]) * ratio);
    viewdir[Z] += ((destdir[Z] - viewdir[Z]) * ratio);
    
  } else {
    this->setCameraToDestination();
  }  
};

bool Camera::cameraAtDestination() {
  
  return (viewpos[X] == destpos[X] &&
	  viewpos[Y] == destpos[Y] &&
	  viewpos[Z] == destpos[Z]);
  
};

bool Camera::cameraNearDestination() {
  
  return ((sqr(destpos[X] - viewpos[X]) +
	   sqr(destpos[Y] - viewpos[Y]) +
	   sqr(destpos[Z] - viewpos[Z])) < 1.0);

  
};

void Camera::mouseMoveCamera(int dx, int dy, bool shift_down) {

  if (viewmode == VIEW_TOPDOWN || viewmode == VIEW_STARCRAFT) {
    
    if (shift_down) {
      float move_vector[3] = {static_cast<float>(-dy), 0.0, static_cast<float>(dx)};

      moveInDirection(1.0, move_vector);
      }

  } else {
  
    float linear_movement = 0.002 * dy;
    float rotational_movement = -0.002 * dx;
    
    float new_destdir[3];
    
    new_destdir[X] = cosf(rotational_movement)*destdir[X] - sinf(rotational_movement)*destdir[Z];
    new_destdir[Z] = sinf(rotational_movement)*destdir[X] + cosf(rotational_movement)*destdir[Z];

    
    destdir[X] = new_destdir[X];
    destdir[Z] = new_destdir[Z];
    
    destpos[X] = viewpos[X] + (viewdir[X] * linear_movement);
    destpos[Y] = viewpos[Y] + (shift_down ? 0 : (viewdir[Y] * linear_movement));
    destpos[Z] = viewpos[Z] + (viewdir[Z] * linear_movement);
    
    setCameraToDestination();
  }
  
};

void Camera::mouseRotateCamera(int dx, int dy) {

  if (viewmode == VIEW_TOPDOWN || viewmode == VIEW_STARCRAFT) {

  } else {

    float horiz_sensitivity = horizontal_rotation_speed;
    float vert_sensitivity = vertical_rotation_speed;
  
    float horizontal_rotation = horiz_sensitivity * dx;
    float vertical_rotation = vert_sensitivity * dy;
  
    current_vertical_rotation += vertical_rotation;

    if (current_vertical_rotation > 1.0) {
      vertical_rotation -= (current_vertical_rotation - 1.0);
      current_vertical_rotation = 1.0;
    }
  
    if (current_vertical_rotation < -1.0) {
      vertical_rotation += (current_vertical_rotation + 1.0);
      current_vertical_rotation = -1.0;
    }
  
    float new_dest_dir[3];    

    float axis[3];
    axis[X] = destdir[Y]*up[Z] - destdir[Z]*up[Y];
    axis[Y] = destdir[Z]*up[X] - destdir[X]*up[Z];
    axis[Z] = destdir[X]*up[Y] - destdir[Y]*up[X];
    float axis_len = dist(axis[X], axis[Y], axis[Z]);
    axis[X] /= axis_len;
    axis[Y] /= axis_len;
    axis[Z] /= axis_len;

    if (-vert_sensitivity < vertical_rotation && vertical_rotation < vert_sensitivity) {
    } else {
#ifdef VERBOSE
      std::cout << "--vert--" << std::endl;
#endif
      rotateCameraAxis(vertical_rotation, axis);
    }
  
    if (-horiz_sensitivity < horizontal_rotation && horizontal_rotation < horiz_sensitivity) {
    } else {
#ifdef VERBOSE
      std::cout << "--horiz--" << std::endl;
#endif
      rotateCameraAxis(horizontal_rotation, up);
    }
  }
  setCameraToDestination();
};

void Camera::rotateCameraAxis(float angle_, float * axis_) {
  Quaternion temp, temp_conj, quat_view, result;
  
#ifdef VERBOSE
  // debug
  std::cout << "axis=(" << axis_[X] << "," << axis_[Y] << "," << axis_[Z] << ")" << std::endl;
  std::cout << "angle=" << angle_ << std::endl;
#endif
  
  temp.x = axis_[X] * sin(angle_/2);
  temp.y = axis_[Y] * sin(angle_/2);
  temp.z = axis_[Z] * sin(angle_/2);
  temp.w = cos(angle_/2);
  
  quat_view.x = destdir[X];
  quat_view.y = destdir[Y];
  quat_view.z = destdir[Z];
  quat_view.w = 0;
  
  temp_conj = temp.getConjugate();
  
#ifdef VERBOSE
  // temp DEBUG
  std::cout << "temp="; temp.print();
  std::cout << "quat_view="; quat_view.print();
  std::cout << "temp_conj="; temp_conj.print();
  // END DEBUG
#endif
  
  result = (temp * quat_view) * temp_conj;
  
#ifdef VERBOSE
  // single line DEBUG
  std::cout << "result="; result.print();
#endif

  // checks if the camera is moving too close to directly up/down
  //  which breaks the quaternion (does not let the camera venture
  //  more than 1 radian away from horizon)
  float up_dir_dot = dot(result, up);
#ifdef VERBOSE
  std::cout << "dot::" << up_dir_dot << std::endl;
#endif
  if (fabsf(up_dir_dot) > 0.7) {
#ifdef VERBOSE
    std::cout << "dot_to_high" << std::endl;
#endif
    return;
  }

  destdir[X] = result.x;
  destdir[Y] = result.y;
  destdir[Z] = result.z;

};

void Camera::rotate2D(float * pivot, float * point, float angle) {

  float new_point[3];
  
  new_point[X] = cosf(angle)*(point[X]-pivot[X]) - sinf(angle)*(point[Y]-pivot[Y]) + pivot[X]; 
  new_point[Y] = point[Y]; // since Y is up
  new_point[Z] = sinf(angle)*(point[Z]-pivot[Z]) + cosf(angle)*(point[Z]-pivot[Z]) + pivot[Z]; 
 
  for (int i = 0; i < 3; ++i) {
    point[i] = new_point[i];
  }
  
};

void Camera::normaliseViewDir() {

  VectorMaths::normalise(viewdir);
    
};

void Camera::normaliseDestDir() {
  
  VectorMaths::normalise(destdir);
  
};

void Camera::printLocation() {
  std::cout << "(" << viewpos[X] << "," << viewpos[Y] << "," << viewpos[Z] << ")->"
	    << "(" << viewdir[X] << "," << viewdir[Y] << "," << viewdir[Z] << ") [" 
	    << dist(viewpos[X], viewpos[Y], viewpos[Z]) << "]->[" 
	    << dist(viewdir[X], viewdir[Y], viewdir[Z]) << "]" << std::endl;
  
};


void Camera::moveForward() {
  if (fix_to_point) {

  } else {
    moveInDirection(this->movement_speed, viewdir);
  }
};

void Camera::moveBackward() {
  if (fix_to_point) {

  } else {
    moveInDirection(-1.0f * this->movement_speed, viewdir);
  }
};

void Camera::strafeLeft() {
  float * left_vector = (float*)malloc(sizeof(float) * 3);
  left_vector[X] = up[Y]*viewdir[Z] - up[Z]*viewdir[Y];
  left_vector[Y] = up[Z]*viewdir[X] - up[X]*viewdir[Z];
  left_vector[Z] = up[X]*viewdir[Y] - up[Y]*viewdir[X];

  VectorMaths::normalise(left_vector);  

  if (fix_to_point) {
    orbitInDirection(this->movement_speed, left_vector);
  } else {
    moveInDirection(this->movement_speed, left_vector);
  }

  free(left_vector);
};

void Camera::strafeRight() {

  float * right_vector = (float*)malloc(sizeof(float) * 3);

  right_vector[X] = viewdir[Y]*up[Z] - viewdir[Z]*up[Y];
  right_vector[Y] = viewdir[Z]*up[X] - viewdir[X]*up[Z];
  right_vector[Z] = viewdir[X]*up[Y] - viewdir[Y]*up[X];
  
  VectorMaths::normalise(right_vector);
  
  if (fix_to_point) {
    orbitInDirection(this->movement_speed, right_vector);
  } else {
    moveInDirection(this->movement_speed, right_vector);
  }

  free(right_vector);
};

/* moves "up" relative to the direction you are facing */
void Camera::strafeUp() {

  float right_vector[3] = {viewdir[Y]*up[Z] - viewdir[Z]*up[Y],
			   viewdir[Z]*up[X] - viewdir[X]*up[Z],
			   viewdir[X]*up[Y] - viewdir[Y]*up[X]};
    

  float rel_up_vector[3] = {right_vector[Y]*viewdir[Z] - right_vector[Z]*viewdir[Y],
			    right_vector[Z]*viewdir[X] - right_vector[X]*viewdir[Z],
			    right_vector[X]*viewdir[Y] - right_vector[Y]*viewdir[X]};
  
  moveInDirection(this->movement_speed, rel_up_vector);
};

/* moves "down" relative to the direction you are facing */
void Camera::strafeDown() {
  
  float right_vector[3] = {viewdir[Y]*up[Z] - viewdir[Z]*up[Y],
			   viewdir[Z]*up[X] - viewdir[X]*up[Z],
			   viewdir[X]*up[Y] - viewdir[Y]*up[X]};
  
  float rel_down_vector[3] = {viewdir[Y]*right_vector[Z] - viewdir[Z]*right_vector[Y],
			    viewdir[Z]*right_vector[X] - viewdir[X]*right_vector[Z],
			    viewdir[X]*right_vector[Y] - viewdir[Y]*right_vector[X]};
  
  moveInDirection(this->movement_speed, rel_down_vector);  
};

void Camera::strafeForward() {
  
  // hardcoded for the time being, if you need to use this in non-VIEW_TOPDOWN, VIEW_STARCRAFT
  // (assumes you are facing a direction that lies on the x, y plane)
  
  float forward_vector[3] = {1.0, 0.0, 0.0};
  moveInDirection(this->movement_speed, forward_vector);
  
};

void Camera::strafeBackward() {

  // hardcoded for the time being, if you need to use this in non-VIEW_TOPDOWN, VIEW_STARCRAFT
  // (assumes you are facing a direction that lies on the x, y plane)
  
  float forward_vector[3] = {-1.0, 0.0, 0.0};
  moveInDirection(this->movement_speed, forward_vector);

};

/* moves "up" relative to the world defined "up", regardless of where you are facing */
void Camera::ascend() {
  float up_vector[3] = {up[X], up[Y], up[Z]};
  if (fix_to_point) {

  } else {
    moveInDirection(this->movement_speed, up_vector);    
  }
};

/* moves "down" relative to the world defined "down", regardless of where you are facing */
void Camera::descend() {
  float down_vector[3] = {up[X], -1*up[Y], up[Z]};
  
  if (fix_to_point) {

  } else {
    moveInDirection(this->movement_speed, down_vector);
  }
};

void Camera::moveInDirection(float d_, float * dir_) {
#ifdef VERBOSE
  std::cout << "DEBUG::moving in direction (" << dir_[X] << "," << dir_[Y] << "," << dir_[Z] << ") by " << d_ << std::endl;
#endif 
  
  destpos[X] += (dir_[X] * d_);
  destpos[Y] += (dir_[Y] * d_);
  destpos[Z] += (dir_[Z] * d_);
  
  setCameraToDestination();
};

void Camera::orbitInDirection(float d_, float * dir_) {
  float original_dist_from_point = dist(destpos[X] - current_orbit_center_point[X],
					 destpos[Y] - current_orbit_center_point[Y],
					 destpos[Z] - current_orbit_center_point[Z]);
  
  destpos[X] += (dir_[X] * d_);
  destpos[Y] += (dir_[Y] * d_);
  destpos[Z] += (dir_[Z] * d_); 
  
  float new_dist_from_point = dist(destpos[X] - current_orbit_center_point[X], 
				   destpos[Y] - current_orbit_center_point[Y],
				   destpos[Z] - current_orbit_center_point[Z]);
  
  float ratio_of_new_to_original = new_dist_from_point / original_dist_from_point;
  
  destpos[X] *= ratio_of_new_to_original;
  destpos[Y] *= ratio_of_new_to_original;
  destpos[Z] *= ratio_of_new_to_original;
  
  destdir[X] = current_orbit_center_point[X] - destpos[X];
  destdir[Y] = current_orbit_center_point[Y] - destpos[Y];
  destpos[Z] = current_orbit_center_point[Z] - destpos[Z];
  
  setCameraToDestination();
  
};

void Camera::jumpToOrigin() {
  
  this->setDestPos(0, 0, 0);
  //  this->setCameraToDestination();
  
};

void Camera::toggleFixToPoint() {  
  fix_to_point = !fix_to_point;
  
  if (fix_to_point) {
    current_orbit_center_point[X] = destpos[X] + (destdir[X] * fix_to_point_look_ahead_dist);
    current_orbit_center_point[Y] = destpos[Y] + (destdir[Y] * fix_to_point_look_ahead_dist);
    current_orbit_center_point[Z] = destpos[Z] + (destdir[Z] * fix_to_point_look_ahead_dist);
  }
  
  std::cout << "FixToOrigin=" << (fix_to_point ? "true" : "false") << std::endl;
  
};

void Camera::gotoViewMode(int m) {
  
  while (viewmode != m) {
    this->nextViewMode();
  }

  
};

void Camera::nextViewMode() {
  int prevviewmode = viewmode;
  
  ++viewmode;
  if (viewmode >= VIEW_TOTAL) viewmode = 0;

  if (viewmode == VIEW_TOPDOWN) {
    /*
     * Can't actually look straight down because the camera works with
     * quaternions, which, somewhere on the line, you would be doing
     * a cross product with the perfect up and perfect down vectors... giving OpenGL darkness
     * (DO NOT: look directly 'up' or directly 'down' (up * -1))
     */
    
    destdir[X] = 0.01;
    destdir[Y] = -0.9999499987;
    destdir[Z] = 0.0;
    
    destpos[X] -= 1.5;
  } else if (viewmode == VIEW_STARCRAFT) {
    destdir[X] = 0.70710678; // 45degs
    destdir[Y] = -0.70710678;
    destdir[Z] = 0.0;
  
    destpos[X] -= 1.5;
  } else if (prevviewmode == VIEW_TOPDOWN || prevviewmode == VIEW_STARCRAFT) {
    destdir[X] = 0.70710678; // 45 degs
    destdir[Y] = -0.7070678;
    destdir[Z] = 0.0;

    destpos[X] += 1.5;
  }

  updateLocation();
  

};



void Camera::followUp() {

  float vert_rotation = -5 * vertical_rotation_speed;

  float axis[3];
  axis[X] = followdir[Y]*up[Z] - followdir[Z]*up[Y];
  axis[Y] = followdir[Z]*up[X] - followdir[X]*up[Z];
  axis[Z] = followdir[X]*up[Y] - followdir[Y]*up[X];
  float axis_len = dist(axis[X], axis[Y], axis[Z]);
  axis[X] /= axis_len;
  axis[Y] /= axis_len;
  axis[Z] /= axis_len;

  orbitCameraAxis(vert_rotation, axis);

};

void Camera::followDown() {

  float vert_rotation = 5 * vertical_rotation_speed;

  float axis[3];
  axis[X] = followdir[Y]*up[Z] - followdir[Z]*up[Y];
  axis[Y] = followdir[Z]*up[X] - followdir[X]*up[Z];
  axis[Z] = followdir[X]*up[Y] - followdir[Y]*up[X];
  float axis_len = dist(axis[X], axis[Y], axis[Z]);
  axis[X] /= axis_len;
  axis[Y] /= axis_len;
  axis[Z] /= axis_len;

  orbitCameraAxis(vert_rotation, axis);

};

void Camera::followLeft() {

  float horiz_rotation = 5 * horizontal_rotation_speed;

  orbitCameraAxis(horiz_rotation, up);

};

void Camera::followRight() {

  float horiz_rotation = -5 * horizontal_rotation_speed;

  orbitCameraAxis(horiz_rotation, up);

};



void Camera::orbitCameraAxis(float angle_, float * axis_) {
  Quaternion temp, temp_conj, quat_view, result;

  temp.x = axis_[X] * sin(angle_/2);
  temp.y = axis_[Y] * sin(angle_/2);
  temp.z = axis_[Z] * sin(angle_/2);
  temp.w = cos(angle_/2);

  quat_view.x = followdir[X];
  quat_view.y = followdir[Y];
  quat_view.z = followdir[Z];
  quat_view.w = 0;

  temp_conj = temp.getConjugate();

  result = (temp * quat_view) * temp_conj;

  followdir[X] = result.x;
  followdir[Y] = result.y;
  followdir[Z] = result.z;

  

};



void Camera::increaseFollowDist() {

  this->followdist *= 1.1f;

};


void Camera::decreaseFollowDist() {

  this->followdist /= 1.1f;

};

void Camera::togglePursuitMode() {
	manualPursuit = !manualPursuit;
}; 
