#include "CarMenuData.h"

namespace OpenNFS {
    CarMenuData::CarMenuData(NFSVersion _nfsVersion, std::string const &carId) : nfsVersion(_nfsVersion), id(carId), carName(carId) {
    }

    bool CarMenuData::LoadMenuData(LibOpenNFS::NFS3::FedataFile &fedataFile) {
        carName = fedataFile.carName;
        colorNames = fedataFile.primaryColourNames;

        return true;
    }

    bool CarMenuData::LoadMenuData(LibOpenNFS::NFS4::FedataFile &fedataFile) {
        carName = fedataFile.menuName;
        colorNames = fedataFile.primaryColourNames;

        return true;
    }

} // namespace OpenNFS