#pragma once
#include <LaserClient.h>

using namespace std;

// Define helper functions
int hexToDec(char number, int pos);

// Constructor that instantiates basic variables
LaserClient::LaserClient()
{
	ConnectSocket = INVALID_SOCKET;
	result = NULL;
	baseAngle = 0;
	stepSize = 0;
	numData = 0;
};

// Destructor which will close the connection with the weeder
LaserClient::~LaserClient()
{
	closesocket(ConnectSocket);
	WSACleanup();
};												// Default destructor - leave as is

// Function that will initialise winsock with the laser port
int LaserClient::initialiseWinsock()
{
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		printf("Winsock failed to initialise: %d\n", WSAGetLastError());
		return iResult;
	};

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	iResult = getaddrinfo(WEEDER_ADDRESS, LASER_PORT, &hints, &result);
	if (iResult != 0)
	{
		printf("Address could not be obtained: %d\n", WSAGetLastError());
		WSACleanup();
	};

	return iResult;
};

// Function that will create a socket
int LaserClient::createSocket()
{
	ConnectSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ConnectSocket == INVALID_SOCKET)
	{
		printf("Failed to create Socket: %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	return 0;
};

// Function that connects the socket to the laser port/weeder
int LaserClient::connectSocket()
{
	iResult = connect(ConnectSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		printf("Socket failed to connect: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		ConnectSocket = INVALID_SOCKET;
		WSACleanup();
	}

	freeaddrinfo(result);

	return iResult;
};

// Function that sends a zid to the user to authenticate connection
char* LaserClient::authenticateClient()
{
	sendbuf = "5165456\n";
	iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
	if (iResult == SOCKET_ERROR)
	{
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
	};
	iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
	return recvbuf;
};

// Function that sends message to Lidar that will return the current laser range data
char* LaserClient::sendReceiveMessage()
{
	sendbuf = "sRN LMDscandata\n";
	iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
	if (iResult == SOCKET_ERROR)
	{
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
	};

	iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
	return recvbuf;
};

// Function that parses through most recent laser range data to return x, y and number of data points
void LaserClient::parseLaser()
{
	// Initialise variables
	int spaceCounter = 0;
	int messageCounter = 0;
	double tempDist = 0;
	int lengthCounter = 0;
	int tempCounter = 0;

	baseAngle = 0;
	stepSize = 0;
	numData = 0;

	std::fill(std::begin(xPoints), std::end(xPoints), 0);
	std::fill(std::begin(yPoints), std::end(yPoints), 0);

	// Count the number of spaces to ensure that we arrive at the relevant data point
	while (spaceCounter < 23)
	{
		if (recvbuf[messageCounter] == ' ')
		{
			spaceCounter++;
		}
		messageCounter++;
	};

	// Create a temporary counter that will hold the length of the base angle value
	tempCounter = messageCounter;
	while (recvbuf[tempCounter] != ' ')
	{
		lengthCounter++;
		tempCounter++;
	};

	// Store the base angle as a decimal number
	while (recvbuf[messageCounter] != ' ')
	{

		baseAngle += hexToDec(recvbuf[messageCounter], lengthCounter);
		messageCounter++;
		lengthCounter--;
	};

	messageCounter++;

	// Create a temporary counter that will hold the length of the step size value
	tempCounter = messageCounter;
	while (recvbuf[tempCounter] != ' ')
	{
		lengthCounter++;
		tempCounter++;
	};

	// Store the step size as a decimal number
	while (recvbuf[messageCounter] != ' ')
	{
		stepSize += hexToDec(recvbuf[messageCounter], lengthCounter);
		messageCounter++;
		lengthCounter--;
	};

	messageCounter++;

	// Create a temporary counter that will hold the length of the number of data points value
	tempCounter = messageCounter;
	while (recvbuf[tempCounter] != ' ')
	{
		lengthCounter++;
		tempCounter++;
	};

	// Store the number of data points as a decimal number
	while (recvbuf[messageCounter] != ' ')
	{
		numData += hexToDec(recvbuf[messageCounter], lengthCounter);
		messageCounter++;
		lengthCounter--;
	};

	messageCounter++;
	
	// Initialise a data interator
	int data = 0;

	// Use trigonometry to store each distance as x and ys while there is still unread distances
	while (data < numData)
	{
		tempCounter = messageCounter;
		while (recvbuf[tempCounter] != ' ')
		{
			lengthCounter++;
			tempCounter++;
		};
		while (recvbuf[messageCounter] != ' ')
		{
			tempDist += hexToDec(recvbuf[messageCounter], lengthCounter);
			messageCounter++;
			lengthCounter--;
		};
		xPoints[data] = tempDist * sin((data * stepSize / 10000) * (PI / 180));
		yPoints[data] = tempDist * cos((data * stepSize / 10000) * (PI / 180));
		tempDist = 0;
		data++;
		messageCounter++;
	};


};

// Getter function for X coordinates
double* LaserClient::getXData()
{
	return xPoints;
}

// Getter function for Y coordinates
double* LaserClient::getYData()
{
	return yPoints;
}

// Getter function for number of data points
double LaserClient::getNumData()
{
	return numData;
}

// Helper function which takes in a hex number in a char array and returns its decimal value
int hexToDec(char number, int pos)
{
	int temp = 0;
	if (number >= '0' && number <= '9') {
		if (pos == 1)
		{
			temp += (number - 48);
		}
		else if (pos == 2)
		{
			temp += (number - 48) * 16;
		}
		else if (pos == 3)
		{
			temp += (number - 48) * 16 * 16;
		}
		else if (pos == 4)
		{
			temp += (number - 48) * 16 * 16 * 16;
		}
		else
		{
			printf("\nIncorrect pos ");
			std::cout << pos << std::endl;
		};
	}
	else if (number >= 'A' && number <= 'F')
	{
		if (pos == 1)
		{
			temp += (number - 55);
		}
		else if (pos == 2)
		{
			temp += (number - 55) * 16;
		}
		else if (pos == 3)
		{
			temp += (number - 55) * 16 * 16;
		}
		else if (pos == 4)
		{
			temp += (number - 55) * 16 * 16 * 16;
		}
		else
		{
			printf("Incorrect pos");
		};
	}

	return temp;
}