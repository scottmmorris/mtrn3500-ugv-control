#include "HUD.hpp"
#include "Camera.hpp"
#include "Vehicle.hpp"

#ifdef __APPLE__
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
	#include <GLUT/glut.h>
	#include <unistd.h>
#elif defined(WIN32)
	#include <Windows.h>
	#include <tchar.h>
	#include <GL/gl.h>
	#include <GL/glu.h>
	#include <GL/glut.h>

	#pragma warning(disable : 4996)
#else
	#include <GL/gl.h>
	#include <GL/glu.h>
	#include <GL/glut.h>
	#include <unistd.h>
#endif

#include <stdio.h>
#include <math.h>

#include <map>


extern Vehicle * vehicle;

using namespace scos;


void HUD::RenderString(const char * str, int x, int y, void * font) 
{
	for (int i = 0; i < strlen(str); i++) {
		glRasterPos2i(x, y);
		glutBitmapCharacter(font, str[i]);
		x += glutBitmapWidth(font, str[i]);
	}
}
#define DEGTORAD (3.141592765 / 180.0)
void HUD::DrawGauge(double x, double y, double r, double min, double max, double val, const char * label, const char * minLabel, const char * maxLabel)
{
	glPushMatrix();
	double r1 = r;
	double r2 = r * 1.05;

	const double centerR = -90;
	const double startR = centerR - 50;
	const double endR = centerR + 50;

	glTranslatef(x, y, 0);
	glDisable(GL_LIGHTING);

	y = sin((startR) * DEGTORAD);
	// text label
	//renderString(label, strlen(label) * 10 * -.25, -r1 + 20, GLUT_BITMAP_HELVETICA_10);
	RenderString(label, strlen(label) * 10 * -.25, (r1 - 20)*y - 20, GLUT_BITMAP_HELVETICA_10);

	glBegin(GL_LINES);
	for(double ang = startR; ang <= endR; ang += (endR - startR) * .125*.5) {
		double x = cos(ang * DEGTORAD);
		double y = sin(ang * DEGTORAD);

		glVertex2f(r1 * x, r1 * y);
		glVertex2f(r2 * x, r2 * y);
	}
	glEnd();

	// labels
	char buff[80];
	sprintf(buff, "%.0f", min);
	if(minLabel)
		strcpy(buff, minLabel);
	RenderString(buff, (r2 + 5) * cos(startR * DEGTORAD) - 8, (r2+5) * sin(startR * DEGTORAD), GLUT_BITMAP_HELVETICA_12);

	sprintf(buff, "%.0f", max);
	if(maxLabel)
		strcpy(buff, maxLabel);
	RenderString(buff, (r2 + 5) * cos(endR * DEGTORAD) - 8, (r2+5) * sin(endR * DEGTORAD), GLUT_BITMAP_HELVETICA_12);

	sprintf(buff, "%.1f", (max + min) * .5);
	RenderString(buff, (r2 + 5) * cos((startR+endR)*.5 * DEGTORAD) - 10, (r2+3) * sin((startR+endR)*.5 * DEGTORAD), GLUT_BITMAP_HELVETICA_12);

	double valPos = ((val - min) / (max - min));
	valPos = (valPos * (endR - startR)) + startR;
	if(valPos < startR)
		valPos = startR;
	else if(valPos > endR)
		valPos = endR;
	//else
	//	valPos = (int) (valPos*2.0)/2.0;

	// draw the bar
	double barW = 1;
	r1 -= 3;
	r2 += 3;
	glColor3f(1,1,1);
	glBegin(GL_QUADS);
		double x1 = cos((valPos - barW) * DEGTORAD);
		double y1 = sin((valPos - barW) * DEGTORAD);
		double x2 = cos((valPos + barW) * DEGTORAD);
		double y2 = sin((valPos + barW) * DEGTORAD);
		glVertex2f(r1 * x1, r1 * y1);
		glVertex2f(r1 * x2, r1 * y2);
		glVertex2f(r2 * x2, r2 * y2);
		glVertex2f(r2 * x1, r2 * y1);
	glEnd();
	
	// draw text value
	sprintf(buff, "%.1f", val);
	RenderString(buff, strlen(buff) * 18 * -.25, (r1 - 20) * y - 5, GLUT_BITMAP_HELVETICA_18);

	// draw border
	glColor3f(.5 ,.5 ,.5);
	const double margin = 5;
	glBegin(GL_LINE_LOOP);
		x = cos((endR + margin) * DEGTORAD);
		y = sin((endR + margin) * DEGTORAD);
		glVertex2f((r1 - 15) * x, (r1 - 15) * y);

		x = cos((startR - margin) * DEGTORAD);
		y = sin((startR - margin) * DEGTORAD);
		glVertex2f((r1 - 15) * x, (r1 - 15) * y);


		for(double ang = startR - margin; ang <= endR + margin; ang += (endR - startR + (2 * margin)) * .125*.5) {
			double x = cos(ang * DEGTORAD);
			double y = sin(ang * DEGTORAD);

			glVertex2f((r2+15) * x, (r2+15) * y);
		}
	glEnd();

	glPopMatrix();
}

void HUD::Draw()
{
	Camera::get()->switchTo2DDrawing();
	int winWidthOff = (Camera::get()->getWindowWidth() - 800) * .5;
	if(winWidthOff < 0)
		winWidthOff = 0;

	if(vehicle) {
		glColor3f(1, 0, 0);
		DrawGauge(200+winWidthOff, 280, 210, -1, 1, vehicle->getSpeed(), "Speed");
		glColor3f(1, 1, 0);
		DrawGauge(600+winWidthOff, 280, 210, -40, 40, vehicle->getSteering(), "Steer");
	}

	Camera::get()->switchTo3DDrawing();
}