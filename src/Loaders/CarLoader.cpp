#include "CarLoader.h"

/*#include <NFS2/NFS2Loader.h>*/
#include <NFS3/NFS3Loader.h>
/*#include "NFS4/PC/NFS4Loader.h"
#include "NFS4/PS1/NFS4PS1Loader.h"
#include "NFS5/NFS5Loader.h"*/

namespace OpenNFS {
    LibOpenNFS::Car CarLoader::_LoadCarAsset(NFSVersion nfsVersion, const std::string &carName) {
        std::stringstream carPath;
        carPath << RESOURCE_PATH << get_string(nfsVersion);

        switch (nfsVersion) {
        /*case NFSVersion::NFS_2:
            carPath << NFS_2_CAR_PATH << carName;
            return LibOpenNFS::NFS2::Loader<LibOpenNFS::NFS2::PC>::LoadCar(carPath.str(), nfsVersion);
        case NFSVersion::NFS_2_SE:
            carPath << NFS_2_SE_CAR_PATH << carName;
            return LibOpenNFS::NFS2::Loader<LibOpenNFS::NFS2::PC>::LoadCar(carPath.str(), nfsVersion);
        case NFSVersion::NFS_2_PS1:
            carPath << "/" << carName;
            return LibOpenNFS::NFS2::Loader<LibOpenNFS::NFS2::PC>::LoadCar(carPath.str(), nfsVersion);*/
        case NFSVersion::NFS_3:
            carPath << NFS_3_CAR_PATH << carName;
            return LibOpenNFS::NFS3::Loader::LoadCar(carPath.str());
        /*case NFSVersion::NFS_3_PS1:
            carPath << "/" << carName;
            return LibOpenNFS::NFS2::Loader<LibOpenNFS::NFS2::PS1>::LoadCar(carPath.str(), nfsVersion);*/
        /*case NFS_4:
            carPath << NFS_4_CAR_PATH << carName;
            return NFS4::LoadCar(carPath.str(), nfsVersion);
            break;
        case NFS_4_PS1:
            carPath << "/" << carName << ".VIV";
            return NFS4PS1::LoadCar(carPath.str());
            break;
        case MCO:
            carPath << MCO_CAR_PATH << carName;
            return NFS4::LoadCar(carPath.str(), nfsVersion);
            break;
        case NFS_5:
            carPath << NFS_5_CAR_PATH << carName;
            return NFS5::LoadCar(carPath.str());
            break;*/
        default:
            CHECK_F(false, "Unknown car type!");
        }
        __builtin_unreachable();
    }

    std::shared_ptr<Car> CarLoader::LoadCar(NFSVersion nfsVersion, const std::string &carName) {
        LibOpenNFS::Car carAsset{_LoadCarAsset(nfsVersion, carName)};
        // Vehicle names are only encoded in mesh Asset files for NFS 3 and 4, we must rely upon part of the filename for other titles
        auto &carAssetName{carAsset.metadata.name};
        if (carAssetName.empty()) {
            carAssetName = carName;
        }
        return std::make_shared<Car>(carAsset);
    }
} // namespace OpenNFS
