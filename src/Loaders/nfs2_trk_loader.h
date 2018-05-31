//
// Created by Amrik on 24/05/2018.
//

#ifndef OPENNFS3_NFS2_TRK_LOADER_H
#define OPENNFS3_NFS2_TRK_LOADER_H

#include <string>
#include <cstring>
#include <fstream>
#include "../nfs2_data.h"

void dbgPrintVerts(NFS2_SUPERBLOCK *superblocks, long nSuperBlocks, long nBlocks, NFS2_VERT_HIGH *blockReferenceCoords, std::string path,bool printFaces);

class nfs2_trk_loader {
public:
    explicit nfs2_trk_loader(const std::string &track_base_path);
    bool LoadTRK(std::string trk_path);
    bool LoadCOL(std::string col_path);
    ~nfs2_trk_loader() = default;
private:
    // TRK data
    uint32_t nSuperBlocks;
    uint32_t nBlocks;
    NFS2_SUPERBLOCK *superblocks;
    // COL data
    uint32_t nTextures;
    NFS2_TEXTURE_BLOCK *polyToQFStexTable;
    uint32_t nColStructures;
    NFS2_3D_BLOCK *colStructures;
    uint32_t nColStructureReferences;
    NFS2_3D_REF_BLOCK *colStructureRefData;
    uint32_t nCollisionData;
    NFS2_COLLISION_BLOCK *collisionData;
};


#endif //OPENNFS3_NFS2_TRK_LOADER_H
