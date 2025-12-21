#pragma once

#include <string>

#include "../Physics/Car.h"

namespace OpenNFS {
    class CarLoader {
    public:
        static std::shared_ptr<Car> LoadCar(NFSVersion nfsVersion, const std::string &carName);

    private:
        static LibOpenNFS::Car _LoadCarAsset(NFSVersion nfsVersion, const std::string &carName); // TODO: BETTER NAME
    };
} // namespace OpenNFS
