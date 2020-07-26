#pragma once

#include <string>

#include "../Physics/Car.h"

class CarLoader
{
public:
    static std::shared_ptr<Car> LoadCar(NFSVer nfsVersion, const std::string &carName);
};
