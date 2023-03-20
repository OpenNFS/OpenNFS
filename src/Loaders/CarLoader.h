#pragma once

#include <string>

#include "../Physics/Car.h"

class CarLoader {
public:
    static std::shared_ptr<Car> LoadCar(NFSVersion nfsVersion, const std::string &carName);
    static std::vector<CarModel> LoadOBJ(std::string obj_path);
};
