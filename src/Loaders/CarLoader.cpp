#include "CarLoader.h"

#include <NFS2/NFS2Loader.h>
#include <NFS3/NFS3Loader.h>
#include <NFS4/PC/NFS4Loader.h>
/*#include "NFS4/PS1/NFS4PS1Loader.h"
#include "NFS5/NFS5Loader.h"*/

namespace OpenNFS {
    LibOpenNFS::Car CarLoader::_LoadCarAsset(NFSVersion const nfsVersion, std::string const &carName) {
        std::stringstream carBasePath, carOutPath;
        carBasePath << RESOURCE_PATH << magic_enum::enum_name(nfsVersion);
        carOutPath << CAR_PATH << magic_enum::enum_name(nfsVersion) << "/" << carName;

        switch (nfsVersion) {
        case NFSVersion::NFS_2:
            carBasePath << NFS_2_CAR_PATH << carName;
            return LibOpenNFS::NFS2::Loader<LibOpenNFS::NFS2::PC>::LoadCar(carBasePath.str(), carOutPath.str(), nfsVersion);
        case NFSVersion::NFS_2_SE:
            carBasePath << NFS_2_SE_CAR_PATH << carName;
            return LibOpenNFS::NFS2::Loader<LibOpenNFS::NFS2::PC>::LoadCar(carBasePath.str(), carOutPath.str(), nfsVersion);
        case NFSVersion::NFS_2_PS1:
            carBasePath << "/" << carName;
            return LibOpenNFS::NFS2::Loader<LibOpenNFS::NFS2::PC>::LoadCar(carBasePath.str(), carOutPath.str(), nfsVersion);
        case NFSVersion::NFS_3:
            carBasePath << NFS_3_CAR_PATH << carName;
            return LibOpenNFS::NFS3::Loader::LoadCar(carBasePath.str(), carOutPath.str());
        case NFSVersion::NFS_3_PS1:
            carBasePath << "/" << carName;
            return LibOpenNFS::NFS2::Loader<LibOpenNFS::NFS2::PS1>::LoadCar(carBasePath.str(), carOutPath.str(), nfsVersion);
        case NFSVersion::NFS_4:
            carBasePath << NFS_4_CAR_PATH << carName;
            return LibOpenNFS::NFS4::Loader::LoadCar(carBasePath.str(), carOutPath.str(), nfsVersion);
        /*case NFS_4_PS1:
            carPath << "/" << carName << ".VIV";
            return NFS4PS1::LoadCar(carPath.str());
            break;*/
        case NFSVersion::MCO:
            carBasePath << MCO_CAR_PATH << carName;
            return LibOpenNFS::NFS4::Loader::LoadCar(carBasePath.str(), carOutPath.str(), nfsVersion);
        /*case NFS_5:
            carPath << NFS_5_CAR_PATH << carName;
            return NFS5::LoadCar(carPath.str());
            break;*/
        default:
            CHECK_F(false, "Unknown car type!");
        }
        unreachable();
    }

    std::shared_ptr<Car> CarLoader::LoadCar(NFSVersion const nfsVersion, std::string const &carName) {
        LibOpenNFS::Car carAsset{_LoadCarAsset(nfsVersion, carName)};
        // Vehicle names are only encoded in mesh Asset files for NFS 3 and 4, we must rely upon part of the filename for other titles
        if (auto &carAssetName{carAsset.metadata.name}; carAssetName.empty()) {
            carAssetName = carName;
        }
        return std::make_shared<Car>(carAsset, carName);
    }

    std::shared_ptr<CarMenuData> CarLoader::LoadCarMenuData(NFSVersion nfsVersion, std::string const &carId) {
        CarMenuData carMenuData(nfsVersion, carId);
        std::stringstream carBasePath, carOutPath;
        carBasePath << RESOURCE_PATH << magic_enum::enum_name(nfsVersion);
        carOutPath << CAR_PATH << magic_enum::enum_name(nfsVersion) << "/" << carId;

        switch (nfsVersion) {
            case NFSVersion::NFS_3:
                {
                    carBasePath << NFS_3_CAR_PATH << carId;
                    LibOpenNFS::NFS3::FedataFile fedataFile = LibOpenNFS::NFS3::Loader::LoadCarMenuData(carBasePath.str(), carOutPath.str());
                    carMenuData.LoadMenuData(fedataFile);
                }
                break;
            case NFSVersion::NFS_4:
                {
                    carBasePath << NFS_4_CAR_PATH << carId;
                    LibOpenNFS::NFS4::FedataFile fedataFile = LibOpenNFS::NFS4::Loader::LoadCarMenuData(carBasePath.str(), carOutPath.str(), nfsVersion);
                    carMenuData.LoadMenuData(fedataFile);
                }
                break;
            case NFSVersion::MCO:
                {
                    carBasePath << MCO_CAR_PATH << carId;
                    LibOpenNFS::NFS4::FedataFile fedataFile = LibOpenNFS::NFS4::Loader::LoadCarMenuData(carBasePath.str(), carOutPath.str(), nfsVersion);
                    carMenuData.LoadMenuData(fedataFile);
                }
                break;
        }

        return std::make_shared<CarMenuData>(carMenuData);
    }
} // namespace OpenNFS
