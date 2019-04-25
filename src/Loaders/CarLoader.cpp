#include "CarLoader.h"

std::shared_ptr<Car> CarLoader::LoadCar(NFSVer nfs_version, const std::string &car_name) {
    std::stringstream car_path;
    car_path << RESOURCE_PATH << ToString(nfs_version);

    switch (nfs_version) {
        case NFS_2:
            car_path << NFS_2_CAR_PATH << car_name;
            break;
        case NFS_2_SE:
            car_path << NFS_2_SE_CAR_PATH << car_name;
            break;
        case NFS_3:
            car_path << NFS_3_CAR_PATH << car_name;
            break;
        case NFS_3_PS1:
            car_path << "/" << car_name;
            break;
        case NFS_4:
            car_path << NFS_4_CAR_PATH << car_name;
            break;
        case UNKNOWN:
            ASSERT(false, "Unknown car type!");
        default:
            break;
    }

    switch (nfs_version) {
        case NFS_2:
            return NFS2<PC>::LoadCar(car_path.str());
        case NFS_2_SE:
            return NFS2<PC>::LoadCar(car_path.str());
        case NFS_3:
           return NFS3::LoadCar(car_path.str());
        case NFS_3_PS1:
            return NFS2<PS1>::LoadCar(car_path.str());
        case NFS_4:
            return NFS4::LoadCar(car_path.str());
        default:
            ASSERT(false, "Unknown car type!");
            break;
    }

    return std::shared_ptr<Car>();
}