#using <System.dll>

#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <map>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#include <unistd.h>
#include <sys/time.h>
#elif defined(WIN32)
#include <Windows.h>
#include <tchar.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <unistd.h>
#include <sys/time.h>
#endif


#include "Camera.hpp"
#include "Ground.hpp"
#include "KeyManager.hpp"

#include "Shape.hpp"
#include "Vehicle.hpp"
#include "MyVehicle.hpp"

#include "Messages.hpp"
#include "HUD.hpp"

#include <SMStructs.h>
#include <SMObject.h>
#include <tlhelp32.h>
#include <conio.h>
#include <deque>

#define PM_WAIT_TIME 50

void display();
void reshape(int width, int height);
void idle();

void keydown(unsigned char key, int x, int y);
void keyup(unsigned char key, int x, int y);
void special_keydown(int keycode, int x, int y);
void special_keyup(int keycode, int x, int y);

void mouse(int button, int state, int x, int y);
void dragged(int x, int y);
void motion(int x, int y);

void drawLaser();
void addLine(double x, double y);
void plotPoint(double x, double y, double z);
void drawGPS();
void drawGPSLine(double x, double pastx, double y, double pasty, double z, double pastz);

using namespace std;
using namespace scos;

// Used to store the previous mouse location so we can calculate relative mouse movement.
int prev_mouse_x = -1;
int prev_mouse_y = -1;

// vehicle control related variables
Vehicle* vehicle = NULL;
double speed = 0;
double steering = 0;

// Shared Memory global pointer variables
PMHeartbeats* PMHeartbeatsPtr = NULL;
LaserData* LaserDataPtr = NULL;
ControlData* ControlDataPtr = NULL;
GPSData* GPSDataPtr = NULL;

// Global counter to check how long PM has been inresponsive
int PMCounter = 0;

// Global deques to store past GPS data
std::deque<double> NorthingPoints;
std::deque<double> EastingPoints;
std::deque<double> HeightPoints;

int main(int argc, char** argv) {

	const int WINDOW_WIDTH = 800;
	const int WINDOW_HEIGHT = 600;

	glutInit(&argc, (char**)(argv));
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutCreateWindow("MTRN3500 - GL");

	Camera::get()->setWindowDimensions(WINDOW_WIDTH, WINDOW_HEIGHT);

	glEnable(GL_DEPTH_TEST);

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutIdleFunc(idle);

	glutKeyboardFunc(keydown);
	glutKeyboardUpFunc(keyup);
	glutSpecialFunc(special_keydown);
	glutSpecialUpFunc(special_keyup);

	glutMouseFunc(mouse);
	glutMotionFunc(dragged);
	glutPassiveMotionFunc(motion);

	// Setting up shared memory
	TCHAR PM[] = TEXT("PMHeartbeats");
	SMObject PMHeartbeatsSMObj(PM, sizeof(PMHeartbeats));
	PMHeartbeatsSMObj.SMAccess();
	PMHeartbeatsPtr = (PMHeartbeats*)PMHeartbeatsSMObj.pData;

	TCHAR LD[] = TEXT("LaserData");
	SMObject LaserDataSMObj(LD, sizeof(LaserData));
	LaserDataSMObj.SMAccess();
	if (LaserDataSMObj.SMAccessError)
	{
		std::cout << "Error getting access to laser data" << std::endl;
		while (1) {}
	}

	LaserDataPtr = (LaserData*)LaserDataSMObj.pData;

	TCHAR CTRL[] = TEXT("ControlData");
	SMObject ControlDataSMObj(CTRL, sizeof(ControlData));
	ControlDataSMObj.SMAccess();
	ControlDataPtr = (ControlData*)ControlDataSMObj.pData;

	TCHAR GPS[] = TEXT("GPSData");
	SMObject GPSDataSMObj(GPS, sizeof(GPSData));
	GPSDataSMObj.SMAccess();
	GPSDataPtr = (GPSData*)GPSDataSMObj.pData;

	// Set the unresponsive PM Counter to 0
	PMCounter = 0;

	vehicle = new MyVehicle();


	glutMainLoop();

	if (vehicle != NULL) {
		delete vehicle;
	}

	return 0;
}


void display() {
	// -------------------------------------------------------------------------
	//  This method is the main draw routine. 
	// -------------------------------------------------------------------------

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if (Camera::get()->isPursuitMode() && vehicle != NULL) {
		double x = vehicle->getX(), y = vehicle->getY(), z = vehicle->getZ();
		double dx = cos(vehicle->getRotation() * 3.141592765 / 180.0);
		double dy = sin(vehicle->getRotation() * 3.141592765 / 180.0);
		Camera::get()->setDestPos(x + (-3 * dx), y + 7, z + (-3 * dy));
		Camera::get()->setDestDir(dx, -1, dy);
	}
	Camera::get()->updateLocation();
	Camera::get()->setLookAt();

	Ground::draw();

	// draw my vehicle
	if (vehicle != NULL) {
		vehicle->draw();

	}

	// draw HUD
	HUD::Draw();

	// draw laser data
	drawLaser();

	// draw GPS data
	drawGPS();

	glutSwapBuffers();

};

void reshape(int width, int height) {

	Camera::get()->setWindowDimensions(width, height);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
};

double getTime()
{
#if defined(WIN32)
	LARGE_INTEGER freqli;
	LARGE_INTEGER li;
	if (QueryPerformanceCounter(&li) && QueryPerformanceFrequency(&freqli)) {
		return double(li.QuadPart) / double(freqli.QuadPart);
	}
	else {
		static ULONGLONG start = GetTickCount64();
		return (GetTickCount64() - start) / 1000.0;
	}
#else
	struct timeval t;
	gettimeofday(&t, NULL);
	return t.tv_sec + (t.tv_usec / 1000000.0);
#endif
}

void idle() {

	// Check to see if PM has not responded for PM_WAIT_TIME iterations and if not then shut down. Else continue to send signals.
	if (PMHeartbeatsPtr->Heartbeats.Flags.Display == 1)
	{
		PMCounter++;
		if (PMCounter > PM_WAIT_TIME)
		{
			PMHeartbeatsPtr->Shutdown.Flags.Display = 0b1;
			exit(0);
		}
	}
	else
	{
		PMHeartbeatsPtr->Heartbeats.Flags.Display = 0b1;
		PMCounter = 0;
	}

	if (PMHeartbeatsPtr->Shutdown.Flags.Display == 1)
	{
		exit(0);
	}

	// Update the GPS buffers with new GPS readings
	if (NorthingPoints.size() != 10000)
	{
		NorthingPoints.push_back(GPSDataPtr->Northing);
		EastingPoints.push_back(GPSDataPtr->Easting);
		HeightPoints.push_back(GPSDataPtr->Height);
	}
	else
	{
		NorthingPoints.push_back(GPSDataPtr->Northing);
		EastingPoints.push_back(GPSDataPtr->Easting);
		HeightPoints.push_back(GPSDataPtr->Height);
		NorthingPoints.pop_front();
		EastingPoints.pop_front();
		HeightPoints.pop_front();
	}

	if (KeyManager::get()->isAsciiKeyPressed('a')) {
		Camera::get()->strafeLeft();
	}

	if (KeyManager::get()->isAsciiKeyPressed('c')) {
		Camera::get()->strafeDown();
	}

	if (KeyManager::get()->isAsciiKeyPressed('d')) {
		Camera::get()->strafeRight();
	}

	if (KeyManager::get()->isAsciiKeyPressed('s')) {
		Camera::get()->moveBackward();
	}

	if (KeyManager::get()->isAsciiKeyPressed('w')) {
		Camera::get()->moveForward();
	}

	if (KeyManager::get()->isAsciiKeyPressed(' ')) {
		Camera::get()->strafeUp();
	}

	speed = 0;
	steering = 0;

	if (KeyManager::get()->isSpecialKeyPressed(GLUT_KEY_LEFT)) {
		steering = Vehicle::MAX_LEFT_STEERING_DEGS * -1;
	}

	if (KeyManager::get()->isSpecialKeyPressed(GLUT_KEY_RIGHT)) {
		steering = Vehicle::MAX_RIGHT_STEERING_DEGS * -1;
	}

	if (KeyManager::get()->isSpecialKeyPressed(GLUT_KEY_UP)) {
		speed = Vehicle::MAX_FORWARD_SPEED_MPS;
	}

	if (KeyManager::get()->isSpecialKeyPressed(GLUT_KEY_DOWN)) {
		speed = Vehicle::MAX_BACKWARD_SPEED_MPS;
	}

	const float sleep_time_between_frames_in_seconds = 0.025;

	static double previousTime = getTime();
	const double currTime = getTime();
	const double elapsedTime = currTime - previousTime;
	previousTime = currTime;

	// Do a simulation step
	if (vehicle != NULL) {
		vehicle->update(speed, steering, elapsedTime);
		
		// Send the speed and steering of the vehicle to shared memory
		ControlDataPtr->speed = vehicle->getSpeed();
		ControlDataPtr->steering = vehicle->getSteering();
	}

	display();

#ifdef _WIN32 
	Sleep(sleep_time_between_frames_in_seconds * 1000);
#else
	usleep(sleep_time_between_frames_in_seconds * 1e6);
#endif
};

void keydown(unsigned char key, int x, int y) {

	// keys that will be held down for extended periods of time will be handled
	//   in the idle function
	KeyManager::get()->asciiKeyPressed(key);

	// keys that react ocne when pressed rather than need to be held down
	//   can be handles normally, like this...
	switch (key) {
	case 27: // ESC key
		exit(0);
		break;
	case '0':
		Camera::get()->jumpToOrigin();
		break;
	case 'p':
		Camera::get()->togglePursuitMode();
		break;
	}

};

void keyup(unsigned char key, int x, int y) {
	KeyManager::get()->asciiKeyReleased(key);
};

void special_keydown(int keycode, int x, int y) {

	KeyManager::get()->specialKeyPressed(keycode);

};

void special_keyup(int keycode, int x, int y) {
	KeyManager::get()->specialKeyReleased(keycode);
};

void mouse(int button, int state, int x, int y) {

};

void dragged(int x, int y) {

	if (prev_mouse_x >= 0) {

		int dx = x - prev_mouse_x;
		int dy = y - prev_mouse_y;

		Camera::get()->mouseRotateCamera(dx, dy);
	}

	prev_mouse_x = x;
	prev_mouse_y = y;
};

void motion(int x, int y) {

	prev_mouse_x = x;
	prev_mouse_y = y;
};

// Helper Function to draw the current set of data that the lidar is giving
void drawLaser() {
	glPushMatrix();
	vehicle->positionInGL();
	glTranslated(0.5, 0, 0);
	glLineWidth(2.5);
	glColor3f(1.0, 1.0, 1.0);
	glBegin(GL_LINES);
	for (int x = 0; x < LaserDataPtr->numOfData;) {
		addLine(LaserDataPtr->xCoordinates[x]/1000, LaserDataPtr->yCoordinates[x]/1000);
		x++;
	}
	glEnd();
	glPopMatrix();
}

// Helper function to draw a vertical line at a give x, y coordinate
void addLine(double x, double y) {
	glVertex3f(x, 0.0, y);
	glVertex3f(x, 1.0, y);
}

// Helper Function to plot a single point on OpenGL given a set of x, y and z coordinates
void plotPoint(double x, double y, double z) {
	glVertex3f(x, z, y);
}

// Helper Function to draw a line on OpenGL given 2 sets of x, y and z coordinates
void drawGPSLine(double x, double pastx, double y, double pasty, double z, double pastz) {
	glVertex3f(pasty, pastz, pastx);
	glVertex3f(y, z, x);
}

// Helper Function to draw the GPS data points on OpenGL
void drawGPS() {

	glPushMatrix();
	//vehicle->positionInGL();
	glTranslated(0.5, 0, 0);
	glLineWidth(2.5);
	glColor3f(0.0, 0.0, 1.0);
	glBegin(GL_LINES);
	for (int x = 0; x < NorthingPoints.size() - 1;) {
		drawGPSLine(NorthingPoints[x], NorthingPoints[x + 1], EastingPoints[x], EastingPoints[x + 1], HeightPoints[x], HeightPoints[x + 1]);
		x++;
	}
	glEnd();
	glPopMatrix();

}
