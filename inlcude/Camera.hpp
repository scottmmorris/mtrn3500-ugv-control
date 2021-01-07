
/* This Camera class is code provided by Stephen Cossell 
 * (stevec@cse.unsw.edu.au) for educational and research
 * purposes within Mechatronics@UNSW. If you would like
 * to use this class in any other work, it's fine, just
 * let me know.
 *    Thanks
 */

#ifndef SCOS_CAMERA_H
#define SCOS_CAMERA_H

#include <cmath>
#include <iostream>

#include "Quaternion.hpp"
#include "Frustum.hpp"
#include "VectorMaths.hpp"

namespace scos {

  class Camera {
    
  private:
    static Camera * instance;

    static const int X = 0;
    static const int Y = 1;
    static const int Z = 2;

    float horizontal_rotation_speed;
    float vertical_rotation_speed;
    float movement_speed;
    
    // where the camera is now
    //  pos=position, dir=direction
    float viewpos[3];
    float viewdir[3];
    
    // where the camera is moving to
    float destpos[3];
    float destdir[3];

    // where is up
    float up[3];

    // follow direction and distance when following an object
    float followdir[3];
    float followdist;
    
    float shakepos[3];

    bool gradual_transitions;
    bool use_toe_in_method;

    bool fix_to_point;
    float current_orbit_center_point[3];
    float fix_to_point_look_ahead_dist;

    float aperture_degs;
    int window_width, window_height;
    float horizontal_half_FOV_dot_product;

    float near_clipping_plane, far_clipping_plane;
    
    float focal_length;

    inline float sqr(float n) {return (n)*(n);};
    inline float dist(float x_, float y_, float z_) {
      return sqrtf(sqr(x_) + sqr(y_) + sqr(z_));
    };
    float dot(float * a, float * b) {return a[X]*b[X]+a[Y]*b[Y]+a[Z]*b[Z];};
    float dot(float * a, Quaternion b) {return a[X]*b.x+a[Y]*b.y+a[Z]*b.z;};
    float dot(Quaternion a, float * b) {return dot(b,a);};

    void rotateCameraAxis(float angle_, float * axis_);
    void orbitCameraAxis(float angle_, float * axis_);
    
    float current_vertical_rotation;

    float eye_separation;
    float eye_angle_radians;

	bool manualPursuit;

  public:

    static Camera * get();

    static const int VIEW_FREE = 0;
    static const int VIEW_BEHIND = 1;
    static const int VIEW_PLAYER = 2;
    static const int VIEW_TOPDOWN = 3;
    static const int VIEW_STARCRAFT = 4;
    static const int VIEW_TOTAL = 5; // total number of viewpoint options
    int viewmode;

    Camera();
    ~Camera();

    Frustum * frustum;

    void setWindowDimensions(int wind_width, int wind_height);
		int getWindowWidth() const { return window_width; }
		int getWindowHeight() const { return window_height; }

    float getAspectRatio();
    
    // the dot product value between the cam_dir and direction at the left/right edge of the screen
    float getHorizontalHalfFOVDotProduct();

    void setViewPort();
    void setProjection();
    
    void setCameraToDestination();
    
    void setDestPos(float x_, float y_, float z_);
    void setDestDir(float x_, float y_, float z_);

    void setShakePos(float x_, float y_, float z_);
    
    float * getViewPos();
    float * getViewDir();
    float * getUpVector();
    
    float * getFollowDir();
    float getFollowDist();

    void increaseFollowDist();
    void decreaseFollowDist();

    void switchTo2DDrawing();
    void switchTo3DDrawing();
    
    void setLookAt();
    void setLookAt(int eye);
    
    void setEyeSeparation(float sep);
    float getEyeSeparation();

    void setEyeAngle(float angle);
    float getEyeAngle();
    
    void updateLocation();

    bool cameraAtDestination();
    
    bool cameraNearDestination();
    
    void mouseMoveCamera(int dx, int dy, bool shift_down);
    void mouseRotateCamera(int dx, int dy);

    void nextViewMode();
    void gotoViewMode(int m);

    void moveForward();
    void moveBackward();
    
    void strafeLeft();
    void strafeRight();
    void strafeUp();
    void strafeDown();
    void strafeForward();
    void strafeBackward();
    
    void ascend();
    void descend();
    void moveInDirection(float d_, float * dir_); // displaces the camera in the direction 'dir_' by an distance of 'd_'
    void orbitInDirection(float d_, float * dir_); // same as above, but corrects so that the distance from the origin after movement is put back to the original distance before movement... also makes sure that you are looking at the origin after movement

    void rotate2D(float * pivot, float * point, float angle);

    void normaliseViewDir();
    void normaliseDestDir();
    
    void printLocation();

    void jumpToOrigin();

    void toggleFixToPoint();

    void followUp();
    void followDown();
    void followLeft();
    void followRight();
    
	void togglePursuitMode();
	bool isPursuitMode() { return manualPursuit; }
  };

};

#endif // for SCOS_DENSO_CAMERA_H
