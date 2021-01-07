#using <System.dll>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <conio.h>//_kbhit()
#include <SMObject.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <SMStructs.h>
#include <LaserClient.h>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 2048
#define LASER_PORT "23000"
#define WEEDER_ADDRESS "192.168.1.200"
#define PM_WAIT_TIME 50


using namespace System;
using namespace System::Net::Sockets;
using namespace System::Net;
using namespace System::Text;


int main()
{

	// Set up shared memory
	TCHAR LD[] = TEXT("LaserData");
	SMObject LaserDataSMObj(LD, sizeof(LaserData));
	LaserData* LaserDataPtr = NULL;
	LaserDataSMObj.SMAccess();
	LaserDataPtr = (LaserData*)LaserDataSMObj.pData;

	TCHAR PM[] = TEXT("PMHeartbeats");
	SMObject PMHeartbeatsSMObj(PM, sizeof(PMHeartbeats));
	PMHeartbeats* PMHeartbeatsPtr = NULL;
	PMHeartbeatsSMObj.SMAccess();
	PMHeartbeatsPtr = (PMHeartbeats*)PMHeartbeatsSMObj.pData;

	// Ensure that the laser program does not shut down from the beginning
	PMHeartbeatsPtr->Shutdown.Flags.Laser = 0;

	// Initialise a class that will connect to the weeder
	LaserClient* weederLaser = new LaserClient();

	// Initialise winsock for the class
	weederLaser->initialiseWinsock();

	// Create a socket for the class and connect to the Weeder socket
	weederLaser->createSocket();
	weederLaser->connectSocket();

	// Send an authentication message to the Lidar
	weederLaser->authenticateClient();

	// Initialise an iterator and the counter to check how long PM has been unresponsive
	int y = 0;
	int PMCounter = 0;

	// Main loop which will continue as long as the program is not being signalled to shutdown
	while (!PMHeartbeatsPtr->Shutdown.Flags.Laser)
	{
		// Continue with data tranfer if PM is alive, otherwise wait and see if PM has been nonresponsive for long enough to exit
		if (PMHeartbeatsPtr->Heartbeats.Flags.Laser == 0)
		{
			PMCounter = 0;
			PMHeartbeatsPtr->Heartbeats.Flags.Laser = 0b1;

			// Send a message to the Lidar that will receive a response of laser coordinates
			weederLaser->sendReceiveMessage();

			// Parse through the current stored laser data to obtain number of data points and x, y points
			weederLaser->parseLaser();

			// Loop through the obtained data, printing to the screen and storing in shared memory
			for (int i = 0; i < weederLaser->getNumData();)
			{
				std::cout << "X coordinate for " << i << ": " << weederLaser->getXData()[i] << "\n" << std::endl;
				LaserDataPtr->xCoordinates[i] = weederLaser->getXData()[i];
				std::cout << "Y coordinate for " << i << ": " << weederLaser->getYData()[i] << "\n" << std::endl;
				LaserDataPtr->yCoordinates[i] = weederLaser->getYData()[i];
				i++;
			};
			LaserDataPtr->numOfData = weederLaser->getNumData();
		}
		else
		{
			PMCounter++;
			if (PMCounter > PM_WAIT_TIME)
			{
				PMHeartbeatsPtr->Shutdown.Flags.Laser = 0b1;
			}
		}

		Sleep(50);
	};

	// Call the destructor to clean up the connection when finished
	weederLaser->~LaserClient();

	return 0;
}


/*

	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo* result = NULL,
		hints;
	char* sendbuf = "5165456\n";
	char recvbuf[DEFAULT_BUFLEN];
	int iResult;
	int recvbuflen = DEFAULT_BUFLEN;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo(WEEDER_ADDRESS, LASER_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// Create a SOCKET for connecting to server
	ConnectSocket = socket(result->ai_family, result->ai_socktype,
		result->ai_protocol);
	if (ConnectSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	// Connect to server.
	iResult = connect(ConnectSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("connect failed with error: %d\n", iResult);
		closesocket(ConnectSocket);
		ConnectSocket = INVALID_SOCKET;
		return 1;
	}

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return 1;
	}

	// Send an initial buffer
	iResult = send( ConnectSocket, sendbuf, (int)strlen(sendbuf), 0 );
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}

	iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
	if (iResult > 0) {
	}
	else if (iResult == 0) {
		printf("Connection closed\n");
	}
	else {
		printf("recv failed with error: %d\n", WSAGetLastError());
	}

	printf(recvbuf);


		sendbuf = "sRN LMDscandata\n";
		iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
		if (iResult == SOCKET_ERROR) {
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(ConnectSocket);
			WSACleanup();
			return 1;
		}
		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
		}
		else if (iResult == 0) {
			printf("Connection closed\n");
		}
		else {
			printf("recv failed with error: %d\n", WSAGetLastError());
		}
		Console::WriteLine(recvbuf[40]);


	// cleanup
	closesocket(ConnectSocket);
	WSACleanup();*/
