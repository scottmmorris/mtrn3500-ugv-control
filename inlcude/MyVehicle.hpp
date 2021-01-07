#ifndef __MYVEHICLE_HPP__
#define __MYVEHICLE_HPP__


#include "Vehicle.hpp"

class MyVehicle : public Vehicle
{
public:
	MyVehicle();
	virtual void draw();
};

#endif