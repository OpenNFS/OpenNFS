//
// Created by Amrik on 24/05/2018.
//

#ifndef OPENNFS3_NFS2_TRK_LOADER_H
#define OPENNFS3_NFS2_TRK_LOADER_H

#include <string>
#include <cstring>
#include <fstream>
#include "../nfs_data.h"

namespace NFS2 {
    TRACK *trk_loadera(const std::string &track_base_path);
    bool LoadTRK(std::string trk_path, TRACK *track);
    bool LoadCOL(std::string col_path, TRACK *track);
    void dbgPrintVerts(TRACK *track, VERT_HIGH *blockReferenceCoords, const std::string &path, bool printFaces);
}

#endif //OPENNFS3_TRK_LOADER_H
