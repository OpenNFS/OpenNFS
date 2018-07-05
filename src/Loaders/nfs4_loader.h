//
// Created by Amrik on 03/07/2018.
//

#pragma once

#include <sstream>
#include <string>
#include <boost/filesystem/path.hpp>
#include "../Config.h"
#include "../nfs_data.h"
#include "../Util/Utils.h"

using namespace Utils;
using namespace NFS4;

class NFS4_Loader {
public:
    explicit NFS4_Loader(const std::string &track_base_path, std::string *car_name);
    std::vector<CarModel> meshes;
private:
    bool LoadFCE(const std::string fce_path);
};

