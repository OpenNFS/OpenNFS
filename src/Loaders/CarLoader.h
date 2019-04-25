#pragma once

#include <string>
#include "NFS2Loader.h"
#include "NFS3Loader.h"
#include "NFS4Loader.h"
#include "../Physics/Car.h"

class CarLoader {
public:
    static std::shared_ptr<Car> LoadCar(NFSVer nfs_version, const std::string &car_name);
};

