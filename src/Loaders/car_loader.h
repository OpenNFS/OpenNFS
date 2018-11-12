//
// Created by Amrik Sadhra on 19/07/2018.
//

#pragma once

#include "../Physics/Car.h"
#include "nfs2_loader.h"
#include "nfs3_loader.h"
#include "nfs4_loader.h"
#include <string>

class CarLoader {
  public:
    static shared_ptr<Car> LoadCar(NFSVer nfs_version, const std::string &car_name);
};
