#include "Messages.hpp"

#include <iostream>
#include <sstream>

#ifdef _WIN32
#pragma warning(disable : 4996)
#endif

static std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) 
{
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}


static std::vector<std::string> split(const std::string &s, char delim) 
{
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

std::vector<VehicleState> GetVehicleStates(const std::string & str)
{
	std::vector<VehicleState> vehicleStates;

	std::vector<std::string> playerStrs = split(str, ';');
	for(unsigned int i = 0; i < playerStrs.size(); i++) {
		VehicleState vs;

		if(sscanf(playerStrs[i].c_str(), "%d=[%f,%f,%f,%f,%f]", &vs.remoteID, &vs.x, &vs.z, &vs.rotation, &vs.speed, &vs.steering) == 6) {
			vehicleStates.push_back(vs);
		}
	}

	return vehicleStates;
}

ShapeType GetShapeTypeFrom(const std::string & str)
{
	ShapeType ret = UNKNOWN_SHAPE;
	// possible types:  RectangularPrism, TriangularPrism, TrapezoidalPrism, Cylinder
	if(str == "RectangularPrism")
		ret = RECTANGULAR_PRISM;
	else if(str == "TriangularPrism")
		ret = TRIANGULAR_PRISM;
	else if(str == "TrapezoidalPrism")
		ret = TRAPEZOIDAL_PRISM;
	else if(str == "Cylinder")
		ret = CYLINDER;

	return ret;
}

std::string GetShapeTypeFrom(const ShapeType st)
{
	std::string ret;
	switch(st) {
		case RECTANGULAR_PRISM:
			ret = "RectangularPrism"; break;
		case TRIANGULAR_PRISM:
			ret = "TriangularPrism"; break;
		case TRAPEZOIDAL_PRISM:
			ret = "TrapezoidalPrism"; break;
		case CYLINDER:
			ret = "Cylinder"; break;
	}
	return ret;
}


std::vector<VehicleModel> GetVehicleModels(const std::string & str)
{
	std::vector<VehicleModel> vehicleModels;

	std::vector<std::string> vehicleStrs = split(str, '|');
	for(unsigned int i = 0; i < vehicleStrs.size(); i++) {
		VehicleModel vm;
		char garbage;
		std::stringstream vss(vehicleStrs[i]);

		// read ID
		vss >> vm.remoteID >> garbage;		

		// get shape info
		std::string shapeStr;
		std::getline(vss, shapeStr);
		std::vector<std::string> shapeStrs = split(shapeStr,';');

		// handle each shape
		for(unsigned int j = 0; j < shapeStrs.size(); j++) {
			std::stringstream sss(shapeStrs[j]);
			ShapeInit shape;

			// read shapes
			std::string type;
			std::string params;
			std::string place;

			sss >> type >> garbage >> params >> place;
			//std::cout << "type is " << type << ", params are " << params << ", place is " << place << std::endl;

			// read parameters
			shape.type = GetShapeTypeFrom(type);		
			switch(shape.type) {
				case RECTANGULAR_PRISM:
					{
						if(sscanf(params.c_str(), "[%f,%f,%f]", &shape.params.rect.xlen, &shape.params.rect.ylen, &shape.params.rect.zlen) != 3)
							shape.type = UNKNOWN_SHAPE;
						break;
					}
				case TRIANGULAR_PRISM:
					{
						if(sscanf(params.c_str(), "[%f,%f,%f,%f]", &shape.params.tri.alen, &shape.params.tri.blen, &shape.params.tri.angle, &shape.params.tri.depth) != 4)
							shape.type = UNKNOWN_SHAPE;
						break;
					}
				case TRAPEZOIDAL_PRISM:
					{
						if(sscanf(params.c_str(), "[%f,%f,%f,%f,%f]", &shape.params.trap.alen, &shape.params.trap.blen, &shape.params.trap.height, &shape.params.trap.aoff, &shape.params.trap.depth) != 5)
							shape.type = UNKNOWN_SHAPE;
						break;
					}
				case CYLINDER:
					{
						shape.params.cyl.isSteering = false;
						shape.params.cyl.isRolling = false;
						if(sscanf(params.c_str(), "[%f,%f]", &shape.params.cyl.radius, &shape.params.cyl.depth) != 2)
							shape.type = UNKNOWN_SHAPE; 
						break;
					}
				default:
					break;
			
			}

			// read placement
			if(sscanf(place.c_str(), "[%f,%f,%f,%f,%f,%f,%f]", &shape.xyz[0], &shape.xyz[1], &shape.xyz[2], &shape.rotation, &shape.rgb[0], &shape.rgb[1], &shape.rgb[2]) != 7)
				shape.type = UNKNOWN_SHAPE;

			// read extra attributes
			if(shape.type == CYLINDER) {
				while(!sss.eof()) {
					std::string attr;
					sss >> attr;

					char status;
					if(sscanf(attr.c_str(), "wheel=%c", &status) == 1) 
						shape.params.cyl.isRolling = (status == 'T');
					else if(sscanf(attr.c_str(), "steer=%c", &status) == 1)
						shape.params.cyl.isSteering = (status == 'T');
				}
			}


			// add valid shapes
			if(shape.type != UNKNOWN_SHAPE)
				vm.shapes.push_back(shape);

		}

		// add model to the list
		vehicleModels.push_back(vm);

	}
	return vehicleModels;
}


std::vector<int> GetVehicleDisconnects(const std::string & str)
{
	std::vector<int> ret;

	std::stringstream dss(str);
	int count = 0;
	dss >> count;
	for(int i = 0; i < count; i++) {
		int remoteID;
		dss >> remoteID;

		ret.push_back(remoteID);
	}
	return ret;
}

std::vector<ObstacleState> GetObstacles(const std::string & str)
{
	std::vector<ObstacleState> ret;
	std::stringstream ss(str);
	int count = 0;
	ss >> count;
	for(int i = 0; i < count; i++) {
		ObstacleState obs;

		std::string param;
		ss >> param;

		if(sscanf(param.c_str(), "[%f,%f,%f]", &obs.x, &obs.z, &obs.radius) == 3)
			ret.push_back(obs);
		else
			std::cerr << "Error in transmission of obstacle state." << std::endl;
	}
	return ret;
}

GoalState GetGoalState(const std::string & str)
{
	GoalState ret;
	std::stringstream ss(str);

	ss >> ret.x >> ret.z;

	return ret;
}

std::ostream & operator<<(std::ostream & oss, const VehicleState & vs)
{
	oss << vs.remoteID << "=[" << vs.x << "," << vs.z << "," << vs.rotation << "," << vs.speed << "," << vs.steering << "]";
	return oss;
}
std::ostream & operator<<(std::ostream & oss, const VehicleModel & vm)
{
	oss << vm.remoteID << " : ";
	for(int i = 0; i < vm.shapes.size(); i++) {
		// add delimiter
		if(i != 0)
			oss << ";";

		oss << vm.shapes[i];
	}
	return oss;
}
std::ostream & operator<<(std::ostream & oss, const ShapeInit & si)
{
	oss << GetShapeTypeFrom(si.type) <<  " ( [";

	// output params
	switch(si.type) {
		case RECTANGULAR_PRISM:
			oss << si.params.rect.xlen << "," << si.params.rect.ylen << "," << si.params.rect.zlen; break;
		case TRIANGULAR_PRISM:
			oss << si.params.tri.alen << "," << si.params.tri.angle << "," << si.params.tri.blen << "," << si.params.tri.depth; break;
		case TRAPEZOIDAL_PRISM:
			oss << si.params.trap.alen << "," << si.params.trap.aoff << "," << si.params.trap.blen << "," << si.params.trap.depth << "," << si.params.trap.height; break;
		case CYLINDER:
			oss << si.params.cyl.depth << "," << si.params.cyl.radius; break;
	}

	// output pose and color
	oss << "] [" << si.xyz[0] << "," << si.xyz[1] << "," << si.xyz[2] << "," << si.rotation << "," << si.rgb[0] << "," << si.rgb[1] << "," << si.rgb[2] << "]";

	// add extra attributes
	if(si.type == CYLINDER) {
		oss << " wheel=" << (si.params.cyl.isRolling ? 'T' : 'F') << " steer=" <<  (si.params.cyl.isSteering ? 'T' : 'F');
	}

	return oss;

}

// serializes a VehicleState object into a string format "%d=[%f,%f,%f,%f,%f]"
std::string GetVehicleStateStr(const VehicleState & vs)
{
	std::stringstream oss;
	oss << vs;
	return oss.str();
}
std::string GetVehicleModelStr(const VehicleModel & vm)
{
	std::stringstream oss;
	oss << vm;
	return oss.str();
}
std::string GetShapeInitStr(const ShapeInit & si)
{
	std::stringstream oss;
	oss << si;
	return oss.str();
}
