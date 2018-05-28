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
    explicit nfs2_trk_loader(const std::string &trk_path);
    bool LoadTRK(std::string trk_path);
    ~nfs2_trk_loader() = default;
private:
    long unknownHeader[5];
    long nSuperBlocks;
    long nBlocks;

    NFS2_SUPERBLOCK *superblocks;
};


#endif //OPENNFS3_NFS2_TRK_LOADER_H
