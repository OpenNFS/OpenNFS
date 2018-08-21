//
// Created by Amrik Sadhra on 19/07/2018.
//

#pragma once

#include <string>
#include "nfs2_loader.h"
#include "nfs3_loader.h"
#include "nfs4_loader.h"
#include "../Physics/Car.h"

class CarLoader {
public:
    static shared_ptr<Car> LoadCar(const std::string &car_path);
};

