//Compile in a C++ CLR empty project
#using <System.dll>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <conio.h>//_kbhit()
#include <iostream>
#include <SMObject.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <SMStructs.h>
#include <zmq.hpp>
#include <Windows.h>

#include "SMStructs.h"
#include "SMObject.h"

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include <turbojpeg.h>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 2048
#define CAMERA_PORT "23000"
#define WEEDER_ADDRESS "192.168.1.200"
#define PM_WAIT_TIME 1000


using namespace System;
using namespace System::Net::Sockets;
using namespace System::Net;
using namespace System::Text;

void display();
void idle();

GLuint tex;
PMHeartbeats* PMHeartbeatsPtr = NULL;
int PMCounter;

//ZMQ settings
zmq::context_t context(1);
zmq::socket_t subscriber(context, ZMQ_SUB);

int main(int argc, char** argv)
{

	// Set up shared memory
	TCHAR PM[] = TEXT("PMHeartbeats");
	SMObject PMHeartbeatsSMObj(PM, sizeof(PMHeartbeats));
	PMHeartbeatsSMObj.SMAccess();
	PMHeartbeatsPtr = (PMHeartbeats*)PMHeartbeatsSMObj.pData;

	// Set the shutdown status so the Camera Module does not shutdown
	PMHeartbeatsPtr->Shutdown.Flags.Camera = 0;

	// Initialise the PM wait counter to zero
	PMCounter = 0;

	//Define window size
	const int WINDOW_WIDTH = 800;
	const int WINDOW_HEIGHT = 600;

	//GL Window setup
	glutInit(&argc, (char**)(argv));
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutCreateWindow("MTRN3500 - Camera");

	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glGenTextures(1, &tex);

	//Socket to talk to server
	subscriber.connect("tcp://192.168.1.200:26000");
	subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0);

	glutMainLoop();

	return 1;
}


void display()
{
	//Set camera as gl texture
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex);

	//Map Camera to window
	glBegin(GL_QUADS);
	glTexCoord2f(0, 1); glVertex2f(-1, -1);
	glTexCoord2f(1, 1); glVertex2f(1, -1);
	glTexCoord2f(1, 0); glVertex2f(1, 1);
	glTexCoord2f(0, 0); glVertex2f(-1, 1);
	glEnd();
	glutSwapBuffers();
}
void idle()
{
	// If the PM Counter has exceeded PM_WAIT_TIME then shutdown, otherwise continue to send signal to PM
	if(PMHeartbeatsPtr->Heartbeats.Flags.Camera == 1)
	{
		PMCounter++;
		if (PMCounter > PM_WAIT_TIME)
		{
			PMHeartbeatsPtr->Shutdown.Flags.Camera = 0b1;
			exit(0);
		}
	}
	else
	{
		PMHeartbeatsPtr->Heartbeats.Flags.Camera = 0b1;
		PMCounter = 0;
	}

	// If Camera Module is being told to shutdown, then exit program
	if (PMHeartbeatsPtr->Shutdown.Flags.Camera == 1)
	{
		exit(0);
	}
	//receive from zmq
	zmq::message_t update;
	if (subscriber.recv(&update, ZMQ_NOBLOCK))
	{
		//Receive camera data
		long unsigned int _jpegSize = update.size();
		std::cout << "received " << _jpegSize << " bytes of data\n";
		unsigned char* _compressedImage = static_cast<unsigned char*>(update.data());
		int jpegSubsamp = 0, width = 0, height = 0;

		//JPEG Decompression
		tjhandle _jpegDecompressor = tjInitDecompress();
		tjDecompressHeader2(_jpegDecompressor, _compressedImage, _jpegSize, &width, &height, &jpegSubsamp);
		unsigned char* buffer = new unsigned char[width * height * 3]; //!< will contain the decompressed image
		printf("Dimensions:  %d   %d\n", height, width);
		tjDecompress2(_jpegDecompressor, _compressedImage, _jpegSize, buffer, width, 0/*pitch*/, height, TJPF_RGB, TJFLAG_FASTDCT);
		tjDestroy(_jpegDecompressor);

		//load texture
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, buffer);
		delete[] buffer;
	}

	display();
}
