//Compile in a C++ CLR empty project
#using <System.dll>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <SMStructs.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <math.h>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 2048
#define LASER_PORT "23000"
#define WEEDER_ADDRESS "192.168.1.200"
#define PI 3.14159265358979323846

using namespace System;
using namespace System::Net::Sockets;
using namespace System::Net;
using namespace System::Text;
using namespace std;

class LaserClient {
	public:
		LaserClient();
		
		~LaserClient();
	
		int initialiseWinsock();

		int createSocket();

		int connectSocket();

		char* authenticateClient();

		char* sendReceiveMessage();

		void parseLaser();

		double* getXData();

		double* getYData();

		double getNumData();

	protected:
		WSADATA wsaData;
		SOCKET ConnectSocket;
		struct addrinfo* result = NULL,
			hints;
		char* sendbuf;
		char recvbuf[DEFAULT_BUFLEN];
		int iResult;
		int recvbuflen = DEFAULT_BUFLEN;
		double baseAngle;
		double stepSize;
		double numData;
		double xPoints[361];
		double yPoints[361];
};

