//
// Created by Amrik on 24/05/2018.
//

#ifndef OPENNFS3_NFS2_DATA_H
#define OPENNFS3_NFS2_DATA_H

#include <cstdint>
#include "nfs_data.h"

typedef struct NFS2_VERT {
    uint16_t x, z, y;
} NFS2_VERT;

typedef struct NFS2_VERT_HIGH {
    uint32_t x, z, y;
} NFS2_VERT_HIGH;

typedef struct NFS2_POLYGONDATA {
    uint16_t texture;
    uint16_t otherSideTex;
    uint8_t vertex[4];
} NFS2_POLYGONDATA;


typedef struct NFS2_TRKBLOCK_HEADER {
    uint32_t blockSize;
    uint32_t blockSizeDup;
    uint16_t nExtraBlocks;
    uint16_t unknown;
    uint32_t blockSerial;
    struct NFS2_VERT_HIGH clippingRect[4];
    uint32_t extraBlockTblOffset;
    uint16_t nStickToNextVerts;
    uint16_t nLowResVert, nMedResVert, nHighResVert;
    uint32_t nLowResPoly, nMedResPoly, nHighResPoly; // Possible uint32_t on PC, and uint16_t on PS1
} NFS2_TRKBLOCK_HEADER;

typedef struct NFS2_EXTRABLOCK {
    uint16_t texture;
    uint16_t otherSideTex;
    uint8_t vertex[4];
} NFS2_EXTRABLOCK;


typedef struct NFS2_TRKBLOCK {
    NFS2_TRKBLOCK_HEADER *header;
    NFS2_VERT *vertexTable;
    NFS2_POLYGONDATA *polygonTable;
    NFS2_EXTRABLOCK *extrablockTable;
} NFS2_TRKBLOCK;

typedef struct NFS2_SUPERBLOCK {
    uint32_t superBlockSize;
    uint32_t nBlocks;
    uint32_t padding;
    NFS2_TRKBLOCK *trackBlocks;
} NFS2_SUPERBLOCK;


#endif //OPENNFS3_NFS2_DATA_H
