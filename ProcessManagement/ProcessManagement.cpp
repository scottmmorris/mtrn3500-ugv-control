#using <System.dll>

#include <SMObject.h>
#include <conio.h>
#include <iostream>
#include <tlhelp32.h>
#include <SMStructs.h>

#define WAIT_AMOUNT 100

using namespace System;
using namespace System::Diagnostics;

int main() {

	// Setting up shared memory
	TCHAR PM[] = TEXT("PMHeartbeats");
	SMObject PMHeartbeatsSMObj(PM, sizeof(PMHeartbeats));
	PMHeartbeats* PMHeartbeatsPtr = NULL;
	PMHeartbeatsSMObj.SMCreate();
	PMHeartbeatsSMObj.SMAccess();
	PMHeartbeatsPtr = (PMHeartbeats*)PMHeartbeatsSMObj.pData;

	TCHAR LD[] = TEXT("LaserData");
	SMObject LaserDataSMObj(LD, sizeof(LaserData));
	LaserData* LaserDataPtr = NULL;
	LaserDataSMObj.SMCreate();
	LaserDataSMObj.SMAccess();
	LaserDataPtr = (LaserData*)LaserDataSMObj.pData;

	TCHAR GPS[] = TEXT("GPSData");
	SMObject GPSDataSMObj(GPS, sizeof(GPSData));
	GPSData* GPSDataPtr = NULL;
	GPSDataSMObj.SMCreate();
	GPSDataSMObj.SMAccess();
	GPSDataPtr = (GPSData*)GPSDataSMObj.pData;

	TCHAR CTRL[] = TEXT("ControlData");
	SMObject ControlDataSMObj(CTRL, sizeof(ControlData));
	ControlData* ControlDataPtr = NULL;
	ControlDataSMObj.SMCreate();
	ControlDataSMObj.SMAccess();
	ControlDataPtr = (ControlData*)ControlDataSMObj.pData;

	// Do not shut down any processes at the startup of the program
	PMHeartbeatsPtr->Shutdown.Status = 0x00;

	// Define a module list containing the processes that need to be called
	array<String^>^ ModuleList = gcnew array<String^>{"Laser.exe", "GNSS.exe", "VehicleControl.exe", "Camera.exe", "OpenGL.exe"};

	array<int>^ Critical = gcnew array<int>(ModuleList->Length) { 1, 0 };

	array<Process^>^ ProcessList = gcnew array<Process^>(ModuleList->Length);

	// Start each process in the list
	for (int i = 0; i < ModuleList->Length - 1; i++)
	{
		ProcessList[i] = gcnew Process();
		ProcessList[i]->StartInfo->FileName = ModuleList[i];
		ProcessList[i]->Start();
	}
	
	// Sleep for one second to allow processes to start up
	Sleep(1000);

	// Start OpenGl a second after to allow data to be transmitted
	ProcessList[4] = gcnew Process();
	ProcessList[4]->StartInfo->FileName = ModuleList[4];
	ProcessList[4]->Start();

	// Set the response counters for each process to 0
	int counterLaser = 0;
	int counterGPS = 0;
	int counterDisplay = 0;
	int counterControl = 0;
	int counterCamera = 0;

	// Enter the main loop until a button is pressed by the user
	while (!_kbhit())
	{
		// Check the heartbeat of the laser module, and shutdown all if unresponsive
		if (PMHeartbeatsPtr->Heartbeats.Flags.Laser == 1)
		{

			PMHeartbeatsPtr->Heartbeats.Flags.Laser = 0b0;
			counterLaser = 0;
		}
		else if (PMHeartbeatsPtr->Shutdown.Flags.Laser == 0)
		{
			counterLaser++;
			if (counterLaser > WAIT_AMOUNT)
			{
				PMHeartbeatsPtr->Shutdown.Status = 0xFF;
				std::cout << "Critical Laser Program Shut down" << std::endl;
			}
		}

		// Check the heartbeat of the camera module, and shutdown all if unresponsive
		if (PMHeartbeatsPtr->Heartbeats.Flags.Camera == 1)
		{
			PMHeartbeatsPtr->Heartbeats.Flags.Camera = 0b0;
			counterCamera = 0;
		}
		else if (PMHeartbeatsPtr->Shutdown.Flags.Camera == 0)
		{
			counterCamera++;
			if (counterCamera > WAIT_AMOUNT)
			{
				PMHeartbeatsPtr->Shutdown.Status = 0xFF;
				std::cout << "Critical Camera Program Shut down" << std::endl;
			}
		}

		// Check the heartbeat of the laser module, and attempt to restart if unresponsive
		if (PMHeartbeatsPtr->Heartbeats.Flags.GPS == 1)
		{
			PMHeartbeatsPtr->Heartbeats.Flags.GPS = 0b0;
			counterGPS = 0;
		}
		else if (PMHeartbeatsPtr->Shutdown.Flags.GPS == 0)
		{
			counterGPS++;
			if (counterGPS > WAIT_AMOUNT*2)
			{
				std::cout << "GPS Program Shut down" << std::endl;
				if (ProcessList[1]->HasExited)
				{
					ProcessList[1]->Start();
				}
				else
				{
					ProcessList[1]->Kill();
					ProcessList[1]->Start();
				}
				counterGPS = 0;
			}
		}

		// Check the heartbeat of the display module, and shutdown all if unresponsive
		if (PMHeartbeatsPtr->Heartbeats.Flags.Display == 1)
		{
			PMHeartbeatsPtr->Heartbeats.Flags.Display = 0b0;
			counterDisplay = 0;
		}
		else if (PMHeartbeatsPtr->Shutdown.Flags.Display == 0)
		{
			counterDisplay++;
			if (counterDisplay > WAIT_AMOUNT*10)
			{
				PMHeartbeatsPtr->Shutdown.Status = 0xFF;
				std::cout << "Critical Display Program Shut down" << std::endl;
			}
		}

		// Check the heartbeat of the control module, and shutdown all if unresponsive
		if (PMHeartbeatsPtr->Heartbeats.Flags.Control == 1)
		{
			PMHeartbeatsPtr->Heartbeats.Flags.Control = 0b0;
			counterControl = 0;
		}
		else if (PMHeartbeatsPtr->Shutdown.Flags.Control == 0)
		{
			counterControl++;
			if (counterControl > WAIT_AMOUNT)
			{
				PMHeartbeatsPtr->Shutdown.Status = 0xFF;
				std::cout << "Critical Camera Program Shut down" << std::endl;
			}
		}

		// If PM has been asked to shutdown, exit the loop
		if (PMHeartbeatsPtr->Shutdown.Flags.PM == 1)
		{
			break;
		}

		Sleep(50);
	}

	// Shutdown all processes if the main loop has been exited
	PMHeartbeatsPtr->Shutdown.Status = 0xFF;

	return 0;
}
