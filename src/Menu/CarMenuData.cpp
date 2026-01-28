#include "CarMenuData.h"

namespace OpenNFS {
    CarMenuData::CarMenuData(NFSVersion _nfsVersion, std::string const &carId) : nfsVersion(_nfsVersion), id(carId), carName(carId) {
    }

    bool CarMenuData::LoadMenuData(LibOpenNFS::NFS3::FedataFile &fedataFile) {
        // TODO: The UIFontAtlas cannot deal with non-ascii characters, so we have to exclude El Niño to prevent a crash
        if (id != "peln" && id != "elni")
          carName = fedataFile.carName;
        colorNames = fedataFile.primaryColourNames;

        return true;
    }

} // namespace OpenNFS