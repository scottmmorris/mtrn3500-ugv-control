//Compile in a C++ CLR empty project
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
#include <iostream>
#include <string.h>
// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 2048
#define CONTROL_PORT 25000
#define WEEDER_ADDRESS "192.168.1.200"
#define PM_WAIT_TIME 50


using namespace System;
using namespace System::Net::Sockets;
using namespace System::Net;
using namespace System::Text;


int main()
{

	// Set up shared memory
	SMObject PMHeartbeatsSMObj(_TEXT("PMHeartbeats"), sizeof(PMHeartbeats));
	PMHeartbeats* PMHeartbeatsPtr = NULL;
	PMHeartbeatsSMObj.SMAccess();
	PMHeartbeatsPtr = (PMHeartbeats*)PMHeartbeatsSMObj.pData;

	TCHAR CTRL[] = TEXT("ControlData");
	SMObject ControlDataSMObj(CTRL, sizeof(ControlData));
	ControlData* ControlDataPtr = NULL;
	ControlDataSMObj.SMAccess();
	ControlDataPtr = (ControlData*)ControlDataSMObj.pData;

	// Ensure that the program is not shutting down immeadiately
	PMHeartbeatsPtr->Shutdown.Flags.Control = 0;

	// Initialise a counter for checking responsiveness of PM
	int PMCounter = 0;

	// Create a pointer to a TcpClient object on managed heap
	TcpClient^ Client;

	// Create arrays of unsigned chars to send and receive data
	array<unsigned char>^ SendData;
	array<unsigned char>^ ReceiveData;

	// Initialise a string to store the response data
	//String^ ResponseData;

	// Create a string that holds a zid to authenticate the control service
	String^ authorisationCommand = gcnew String("5165456\n");

	// Create TcpClient object and connect to it
	Client = gcnew TcpClient(WEEDER_ADDRESS, CONTROL_PORT);

	// Configure the connection
	Client->NoDelay = true;
	Client->ReceiveTimeout = 500;//ms
	Client->SendTimeout = 500;//ms
	Client->ReceiveBufferSize = 1024;
	Client->SendBufferSize = 1024;

	// Create a toggle object which will help to change control flags
	int Toggle = 1;

	// Create pointers to unsigned char arrays on the managed heap (that is a ^ instead of *)
	array<unsigned char>^ SendAuth;
	array<unsigned char>^ SendData1;
	array<unsigned char>^ SendData2;

	// Create arrays of unsigned chars that are 16 bytes long and will help send and receive data
	ReceiveData = gcnew array<unsigned char>(16);
	SendData = gcnew array<unsigned char>(16);

	// Encode the authnetication string into bytes
	SendAuth = System::Text::Encoding::ASCII->GetBytes(authorisationCommand);
	
	// Get the network stream object associated with clien so we can use it to read and write
	NetworkStream^ Stream = Client->GetStream();

	// Write the authentication string to the control service
	Stream->Write(SendAuth, 0, SendAuth->Length);

	// Wait for the data to transfer
	Sleep(10);

	// Fill a buffer with the response from the control service
	Stream->Read(ReceiveData, 0, ReceiveData->Length);

	// Convert the response into a readable format (string) and print
	//ResponseData = System::Text::Encoding::ASCII->GetString(ReceiveData);
	//ResponseData = ResponseData->Replace(":", "");
	//Console::WriteLine(ResponseData);

	// Create two char arrays of 16 bytes which will hold our alternating steering/speed commands
	SendData1 = gcnew array<unsigned char>(16);
	SendData2 = gcnew array<unsigned char>(16);

	// Main loop which will continue as long as the program is not being signalled to shutdown
	while (!PMHeartbeatsPtr->Shutdown.Flags.Control)
	{
		// Continue with data tranfer if PM is alive, otherwise wait and see if PM has been nonresponsive for long enough to exit
		if (PMHeartbeatsPtr->Heartbeats.Flags.Control == 0)
		{
			PMHeartbeatsPtr->Heartbeats.Flags.Control = 1;
			PMCounter = 0;

			// Fill the two strings with the current speed and steering in the correct format although one has a 1 and the other a 0 (invert steering as well)
			String^ Control1 = gcnew String("# " + Convert::ToString(-1 * ControlDataPtr->steering) + " " + Convert::ToString(ControlDataPtr->speed) + " 1 #");
			String^ Control2 = gcnew String("# " + Convert::ToString(-1 * ControlDataPtr->steering) + " " + Convert::ToString(ControlDataPtr->speed) + " 0 #");

			// Change the toggle value
			Toggle = 1 - Toggle;

			// Depending on the value of the toggle, send either the first or second string
			if (Toggle)
			{
				Stream->Write(SendData1, 0, SendData1->Length);
				SendData1 = System::Text::Encoding::ASCII->GetBytes(Control1);
				Console::WriteLine(Control1);
			}
			else
			{
				Stream->Write(SendData2, 0, SendData2->Length);
				SendData2 = System::Text::Encoding::ASCII->GetBytes(Control2);
				Console::WriteLine(Control2);
			}
			
			// Wait for one second
			System::Threading::Thread::Sleep(100);//ms
		}
		else
		{
			PMCounter++;
			if (PMCounter > PM_WAIT_TIME)
			{
				PMHeartbeatsPtr->Shutdown.Flags.Control = 0b1;
			}
		}
		Sleep(50);
	};

	// Clean up connection with control service
	Stream->Close();
	Client->Close();

	return 0;

};
