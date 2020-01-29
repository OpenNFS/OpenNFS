#pragma once

#include <string>
//#include "NFS2/PC/NFS2Loader.h"
#include "NFS3/NFS3Loader.h"
/*#include "NFS4/PC/NFS4Loader.h"
#include "NFS4/PS1/NFS4PS1Loader.h"
#include "NFS5/NFS5Loader.h"*/
#include "../Physics/Car.h"

class CarLoader
{
public:
	static std::shared_ptr<Car> LoadCar(NFSVer nfs_version, const std::string &car_name);
};
