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


#define CRC32_POLYNOMIAL			0xEDB88320L
#pragma pack(1)

#define PORT_OPEN_ERROR 2
#define READ_TIMEOUT 3
#define NO_HEADER 4
#define CRC_ERROR 5
#define DEFAULT_BUFLEN 2048
#define GNSS_PORT 24000
#define WEEDER_ADDRESS "192.168.1.200"
#define PM_WAIT_TIME 50

using namespace System;
using namespace System::IO::Ports;

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")



using namespace System;
using namespace System::Net::Sockets;
using namespace System::Net;
using namespace System::Text;

//Global data
double Northing;
double Easting;
double Height;
int ErrorCode;
GPSData NovatelGPS;

unsigned long CRC32Value(int i)
{
	int j;
	unsigned long ulCRC;
	ulCRC = i;
	for (j = 8; j > 0; j--)
	{
		if (ulCRC & 1)
			ulCRC = (ulCRC >> 1) ^ CRC32_POLYNOMIAL;
		else
			ulCRC >>= 1;
	}
	return ulCRC;
}

/* --------------------------------------------------------------------------
Calculates the CRC-32 of a block of data all at once
-------------------------------------------------------------------------- */
unsigned long CalculateBlockCRC32(unsigned long ulCount, /* Number of bytes in the data block */
	unsigned char* ucBuffer) /* Data block */
{
	unsigned long ulTemp1;
	unsigned long ulTemp2;
	unsigned long ulCRC = 0;
	while (ulCount-- != 0)
	{
		ulTemp1 = (ulCRC >> 8) & 0x00FFFFFFL;
		ulTemp2 = CRC32Value(((int)ulCRC ^ *ucBuffer++) & 0xff);
		ulCRC = ulTemp1 ^ ulTemp2;
	}
	return(ulCRC);
}


int main()
{

	// Set up shared memory
	SMObject PMHeartbeatsSMObj(_TEXT("PMHeartbeats"), sizeof(PMHeartbeats));
	PMHeartbeats* PMHeartbeatsPtr = NULL;
	PMHeartbeatsSMObj.SMAccess();
	PMHeartbeatsPtr = (PMHeartbeats*)PMHeartbeatsSMObj.pData;

	TCHAR GPS[] = TEXT("GPSData");
	SMObject GPSDataSMObj(GPS, sizeof(GPSData));
	GPSData* GPSDataPtr = NULL;
	GPSDataSMObj.SMAccess();
	GPSDataPtr = (GPSData*)GPSDataSMObj.pData;

	// Ensure that the GPS Module is not shutting down at the beginning
	PMHeartbeatsPtr->Shutdown.Flags.GPS = 0;

	// Initialise a buffer that will hold GPS Data
	unsigned char Buffer[sizeof(GPSData)];

	// Initialise Pointer to TcpClent type object on managed heap
	TcpClient^ Client;

	// Initialise an array of unsigned chars to receive data
	array<unsigned char>^ ReadData;

	// Create a TcpClient object and connect to it
	Client = gcnew TcpClient(WEEDER_ADDRESS, GNSS_PORT);

	// Configure the connection
	Client->NoDelay = true;
	Client->ReceiveTimeout = 500;//ms
	Client->SendTimeout = 500;//ms
	Client->ReceiveBufferSize = 1024;
	Client->SendBufferSize = 1024;

	// Initialise an unsigned char array of 300 bytes each are created on managed heap
	ReadData = gcnew array<unsigned char>(300);

	// Get the network stream object associated with client so we can use it to read and write
	NetworkStream^ Stream = Client->GetStream();

	// Set the PM counter to zero
	int PMCounter = 0;

	// Main loop which will continue as long as the program is not being signalled to shutdown
	while (!PMHeartbeatsPtr->Shutdown.Flags.GPS)
	{
		// Continue with data tranfer if PM is alive, otherwise wait and see if PM has been nonresponsive for long enough to exit
		if (PMHeartbeatsPtr->Heartbeats.Flags.GPS == 0)
		{
			PMHeartbeatsPtr->Heartbeats.Flags.GPS = 1;
			PMCounter = 0;
			
			// Check if there is Data availble from the GPS
			if (Stream->DataAvailable)
			{
				// Read in the GPS data to a buffer
				Stream->Read(ReadData, 0, ReadData->Length);

				// Initialise some variables to iterate through GPS data
				unsigned int Header = 0;
				int i = 0;
				int Start;
				unsigned char Data;

				// Finding the start of the data
				do
				{
					Data = ReadData[i++];
					Header = ((Header << 8) | Data);
				} while (Header != 0xaa44121c);

				// Go back to where the header begins (4 bytes)
				Start = i - 4;

				// Initialise a GPS data struct and a pointer that points to the beginning of it
				GPSData  NovatelGPS;
				unsigned char* BytePtr = nullptr;
				BytePtr = (unsigned char*)&NovatelGPS;

				// Iterate through the GPS data and input the relevant fields into the data struct
				for (int i = Start; i < Start + sizeof(GPSData); i++)
				{
					*(BytePtr) = ReadData[i];
					Buffer[i] = ReadData[i];
					BytePtr++;
				}

				// Print out the values received from the GPS
				std::cout << "Northing is: " << NovatelGPS.Northing << "\n" << std::endl;
				std::cout << "Easting is: " << NovatelGPS.Easting << "\n" << std::endl;
				std::cout << "Height is: " << NovatelGPS.Height << "\n" << std::endl;
				std::cout << "Received CRC is: " << NovatelGPS.Checksum << "\n" << std::endl;

				// Calculate the CRC of the the received data
				std::cout << "Calculated CRC is: " << CalculateBlockCRC32(108, Buffer) << "\n" << std::endl;
				
				// If the calculated and received CRC are the same then share the data, otherwise do not share the data
				if (NovatelGPS.Checksum == CalculateBlockCRC32(108, Buffer))
				{
					GPSDataPtr->Northing = NovatelGPS.Northing;
					GPSDataPtr->Easting = NovatelGPS.Easting;
					GPSDataPtr->Height = NovatelGPS.Height;
					GPSDataPtr->Checksum = NovatelGPS.Checksum;
				}
				else
				{
					printf("GPS Data Corrupt!\n");
				}
			}
		}
		else
		{
			PMCounter++;
			if (PMCounter > PM_WAIT_TIME)
			{
				PMHeartbeatsPtr->Shutdown.Flags.GPS = 0b1;
			}
		}
		Sleep(50);
	}

	// Clean up the connection with the client
	Stream->Close();
	Client->Close();

	return 0;
};
