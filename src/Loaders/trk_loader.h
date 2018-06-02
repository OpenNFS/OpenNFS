//
// Created by Amrik on 16/01/2018.
//


#ifndef OPENNFS3_TRK_LOADER_H
#define OPENNFS3_TRK_LOADER_H

#include <sstream>
#include <set>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <cstring>
#include <map>
#include "boost/filesystem.hpp"
#include <glm/vec3.hpp>
#include "../nfs_data.h"
#include "../Scene/TrackBlock.h"
#include "../Scene/Track.h"
#include "../Scene/Model.h"
#include "../Util/Utils.h"
#include "../Util/Raytracer.h"

namespace NFS3 {
    // Implementation
    TRACK *trk_loader(const std::string &track_base_path);

    bool LoadFRD(std::string frd_path, TRACK *track);

    bool LoadCOL(std::string col_path, TRACK *track);

    std::vector<TrackBlock> ParseTRKModels(TRACK *track);

    std::vector<Track> ParseCOLModels(TRACK *track);

    std::map<short, GLuint> GenTrackTextures(std::map<short, Texture> textures);

    std::vector<short> RemapTextureIDs(const std::set<short> &minimal_texture_ids_set,
                                       std::vector<unsigned int> &texture_indices);

    Texture LoadTexture(TEXTUREBLOCK track_texture);
}

namespace NFS2 {
    TRACK *trk_loader(const std::string &track_base_path);
    bool LoadTRK(std::string trk_path, TRACK *track);
    bool LoadCOL(std::string col_path, TRACK *track);
    void dbgPrintVerts(TRACK *track, VERT_HIGHP *blockReferenceCoords, const std::string &path, bool printFaces);
}


#endif //FCE_TO_OBJ_TRK_LOADER_H

