#pragma once

#include <fstream>
#include <string>
#include <cstring>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>


#include "../Physics/Car.h"
#include "../nfs_data.h"
#include "../Util/ImageLoader.h"

using namespace NFS5_DATA;

class NFS5 {
public:
    static std::shared_ptr<Car> LoadCar(const std::string &carBasePath);
    static CarData LoadCRP(const std::string &crpPath);
private:
    static bool DecompressCRP(const std::string &compressedCrpPath, const std::string &decompressedCrpPath);
};


