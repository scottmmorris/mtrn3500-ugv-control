#pragma once

struct ModuleFlags
{
	unsigned char	PM : 1,
		GPS : 1,
		Laser : 1,
		Display : 1,
		Control : 1,
		Camera : 1,
		Unused : 2;
};

union ExecFlags
{
	unsigned char Status;
	ModuleFlags Flags;
};

struct PMHeartbeats
{
	ExecFlags Heartbeats;
	ExecFlags Shutdown;
};

struct LaserData
{
	double numOfData;
	double xCoordinates[361];
	double yCoordinates[361];
};

struct ControlData
{
	double speed;
	double steering;
};

#pragma pack(push, 4)
struct GPSData//112 bytes
{
	unsigned int Header;
	unsigned char Discards1[40];
	double Northing;
	double Easting;
	double Height;
	unsigned char Discards2[40];
	unsigned int Checksum;
};
#pragma pack(pop)