//
// Created by Amrik on 02/06/2018.
//

#ifndef OPENNFS3_MAP_LOADER_H
#define OPENNFS3_MAP_LOADER_H

#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include "../nfs_data.h"

using namespace std;

namespace Music{
    void ParseMAP(const std::string &map_path);
}

#endif //OPENNFS3_MAP_LOADER_H
