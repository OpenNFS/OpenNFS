#pragma once

#include <fstream>
#include <string>
#include <cstring>
#include <glm/gtc/type_ptr.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include "../../Physics/Car.h"
#include "../../nfs_data.h"
#include "../../Util/Utils.h"
#include "../../Util/ImageLoader.h"
#include "../../../include/CrpLib/Lib.h"

using namespace NFS5_DATA;
using namespace CrpLib;
using namespace Utils;

class NFS5 {
public:
    static std::shared_ptr<Car> LoadCar(const std::string &carBasePath);
    static CarData LoadCRP(const std::string &crpPath);
private:
    // OpenNFS derived method of dumping FSH textures. To be Deprecated eventually in favour of CrpLib mechanisms
    static void DumpCrpTextures(const std::string &crpPath);
    static void DumpArticleVertsToObj(CRP::ARTICLE_DATA article);
};


