//
// Created by SYSTEM on 19/07/2018.
//

#include "car_loader.h"

shared_ptr<Car> CarLoader::LoadCar(const std::string &car_path) {
    NFSVer nfs_version;

    if (car_path.find("NFS2_SE") != std::string::npos) {
        nfs_version = NFS_2_SE;
    } else if (car_path.find("NFS2") != std::string::npos) {
        nfs_version = NFS_2;
    } else if (car_path.find("NFS3") != std::string::npos) {
        nfs_version = NFS_3;
    } else if (car_path.find("NFS3_PS1") != std::string::npos) {
        nfs_version = NFS_3_PS1;
    } else if (car_path.find("NFS4") != std::string::npos) {
        nfs_version = NFS_4;
    } else {
        nfs_version = UNKNOWN;
    }

    switch (nfs_version) {
        case NFS_2:
            return NFS2<PC>::LoadCar(car_path);
        case NFS_2_SE:
            return NFS2<PC>::LoadCar(car_path);
        case NFS_3:
           return NFS3::LoadCar(car_path);
        case NFS_3_PS1:
            return NFS2<PS1>::LoadCar(car_path);
        case NFS_4:
            return NFS4::LoadCar(car_path);
        case UNKNOWN:
            ASSERT(false, "Unknown track type!");
        default:
            break;
    }
}