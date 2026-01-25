#pragma once

#include <string>
#include <Common/NFSVersion.h>

#include "../Physics/Car.h"
#include "../Menu/CarMenuData.h"


namespace OpenNFS {
    class CarLoader {
      public:
        static std::shared_ptr<Car> LoadCar(NFSVersion nfsVersion, std::string const &carName);
        static std::shared_ptr<CarMenuData> LoadCarMenuData(NFSVersion nfsVersion, std::string const &carName);

      private:
        static LibOpenNFS::Car _LoadCarAsset(NFSVersion nfsVersion, std::string const &carName); // TODO: BETTER NAME
    };
} // namespace OpenNFS
