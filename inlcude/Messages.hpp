#ifndef MTRN3500_MESSAGES_H
#define MTRN3500_MESSAGES_H

#include <vector>
#include <string>

// --- code for handling vehicle state ---

struct VehicleState
{
	int remoteID;
	float x;
	float z; // y up
	float rotation;
	float speed;
	float steering;
};

// --- code for handling vehicle models ---

enum ShapeType
{
	UNKNOWN_SHAPE,
	RECTANGULAR_PRISM,
	TRIANGULAR_PRISM,
	TRAPEZOIDAL_PRISM,
	CYLINDER
};

union ShapeParameter
{
	struct RectangularParameters
	{
		float xlen;		// length along x-axis
		float ylen;		// length along y-axis
		float zlen;		// length along z-axis
	} rect;

	struct TriangularParameters
	{
		float alen;		// length of side A (bottom)
		float blen;		// length of side B (left)
		float angle;	// angle between side A and B
		float depth;	// length along z-axis
	} tri;

	struct TrapezoidalParameters
	{
		float alen;		// length of side A (bottom)
		float blen;		// length of side B (top)
		float height;	// distance between side A and B 
		float aoff;		// distance A is shifted from B by, from the left
		float depth;	// length along z-axis
	} trap;

	struct CylinderParameters
	{
		float radius;	
		float depth;	// length along z-axis

		bool isRolling;	// needs to roll with vehicle?
		bool isSteering;// needs to steer with vehicle?
	} cyl;
};

struct ShapeInit
{
	ShapeType		type;

	ShapeParameter	params;

	float xyz[3];
	float rotation;
	float rgb[3];
};

struct VehicleModel
{
	int remoteID;
	std::vector<ShapeInit> shapes;
};

std::vector<VehicleState> GetVehicleStates(const std::string & str);
std::vector<VehicleModel> GetVehicleModels(const std::string & str);
std::vector<int> GetVehicleDisconnects(const std::string & str);

std::string GetVehicleStateStr(const VehicleState & vs);
std::string GetVehicleModelStr(const VehicleModel & vm);
std::string GetShapeInitStr(const ShapeInit & si);

std::ostream & operator<<(std::ostream & oss, const VehicleState & vs);
std::ostream & operator<<(std::ostream & oss, const VehicleModel & vs);
std::ostream & operator<<(std::ostream & oss, const ShapeInit & vs);

// --- code for handling obstacles ---

struct ObstacleState
{
	float x;
	float z; 
	float radius;
};
std::vector<ObstacleState> GetObstacles(const std::string & str);

// --- code for handling goal state ---

struct GoalState
{
	float x;
	float z; 
};
GoalState GetGoalState(const std::string & str);


#endif
