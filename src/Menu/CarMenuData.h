#pragma once

#include <string>
#include <vector>

#include <Common/NFSVersion.h>
#include <NFS3/FEDATA/FedataFile.h>
#include <NFS4/PC/FEDATA/FedataFile.h>

namespace OpenNFS {
    class CarMenuData {
      public:
        CarMenuData(NFSVersion _nfsVersion, std::string const &carId);

        bool LoadMenuData(LibOpenNFS::NFS3::FedataFile &fedataFile);
        bool LoadMenuData(LibOpenNFS::NFS4::FedataFile &fedataFile);

        NFSVersion nfsVersion;
        std::string id;
        std::string carName;
        std::vector<std::string> colorNames; 
    };
} // namespace OpenNFS
