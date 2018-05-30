//
// Created by Amrik on 24/05/2018.
//

#ifndef OPENNFS3_NFS2_DATA_H
#define OPENNFS3_NFS2_DATA_H

#include <cstdint>
#include "nfs_data.h"

// ---- CORE DATA TYPES ----
typedef struct NFS2_VERT {
    int16_t x, z, y;
} NFS2_VERT;

typedef struct NFS2_VERT_HIGH {
    int32_t x, z, y;
} NFS2_VERT_HIGH;

typedef struct NFS2_ANIM_POS {
    NFS2_VERT_HIGH position;
    uint16_t unknown[4];
} NFS2_ANIM_POS;

typedef struct NFS2_POLYGONDATA {
    uint16_t texture;
    uint16_t otherSideTex;
    uint8_t vertex[4];
} NFS2_POLYGONDATA;

typedef struct NFS2_VROAD {
    NFS2_VERT normalVec;
    NFS2_VERT forwardVec;
} NFS2_VROAD;

// ----------------- EXTRA BLOCKS -----------------
typedef struct NFS2_EXTRABLOCK_HEADER {
    uint32_t recSize;
    uint16_t XBID;
    uint16_t nRecords;
} NFS2_EXTRABLOCK_HEADER;

// Matches number of NP1 polygons in corresponding trackblock
typedef struct NFS2_POLY_TYPE {
    // XBID = 5
    uint8_t xblockRef; // Refers to an entry in the XBID=13 extrablock
    uint8_t carBehaviour;
} NFS2_POLY_TYPE;

typedef struct NFS2_3D_BLOCK {
    // XBID = 8
    uint32_t recSize;
    uint16_t nVerts;
    uint16_t nPoly;
    NFS2_VERT *vertexTable;
    NFS2_POLYGONDATA *polygonTable;
} NFS2_3D_BLOCK;

typedef struct NFS2_3D_REF_BLOCK {
    // XBID = 7, 18
    uint16_t recSize; // If Anim (recType == 3) , recSize == 8 + 20*animation length
    uint8_t recType;
    uint8_t structureRef;
    // Fixed Type (recType == 1)
    NFS2_VERT_HIGH refCoordinates;
    // Animated Type (recType == 3)
    uint16_t animLength; // num of position records
    uint16_t unknown; // Potentially time between animation steps?
    NFS2_ANIM_POS *animationData; // Sequence of positions which animation follows
} NFS2_3D_REF_BLOCK;


// Matches number of full resolution polygons
typedef struct NFS2_MEDIAN_BLOCK {
    // XBID = 6
    uint8_t refPoly[8];
} NFS2_MEDIAN_BLOCK;


typedef struct NFS2_LANE_BLOCK {
    // XBID = 9
    uint8_t vertRef; // Inside B3D structure: 0 to nFullRes + nStickToNext
    uint8_t trackPos; // Position along track inside block (0 to 7)
    uint8_t latPos; // Lateral position, -1 at the end
    uint8_t polyRef; // Inside Full-res B3D structure, 0 to nFullRes
} NFS2_LANE_BLOCK;

// ---- COL Specific Extra Blocks ----
typedef struct NFS2_TEXTURE_BLOCK {
    // XBID = 9
    uint16_t texNumber; // Texture number in QFS file
    uint16_t alignmentData;
    uint8_t RGB[3]; // Luminosity
    uint8_t RGBlack[3]; // Usually black
} NFS2_TEXTURE_BLOCK;


// ------------ TRACK BLOCKS ----------------
typedef struct NFS2_TRKBLOCK_HEADER {
    uint32_t blockSize;
    uint32_t blockSizeDup;
    uint16_t nExtraBlocks;
    uint16_t unknown;
    uint32_t blockSerial;
    struct NFS2_VERT_HIGH clippingRect[4];
    uint32_t extraBlockTblOffset;
    uint16_t nStickToNextVerts, nLowResVert, nMedResVert, nHighResVert;
    uint16_t nLowResPoly, nMedResPoly, nHighResPoly; // Possible uint32_t on PC, and uint16_t on PS1
    //uint16_t padding;
} NFS2_TRKBLOCK_HEADER;


typedef struct NFS2_TRKBLOCK {
    NFS2_TRKBLOCK_HEADER *header;
    NFS2_VERT *vertexTable;
    NFS2_POLYGONDATA *polygonTable;
    NFS2_POLY_TYPE *polyTypes;
    uint16_t nNeighbours;
    uint16_t *blockNeighbours;
    uint16_t nStructures;
    NFS2_3D_BLOCK *structures;
    uint16_t nStructureReferences;
    NFS2_3D_REF_BLOCK *structureRefData;
    NFS2_MEDIAN_BLOCK *medianData;
    uint16_t nVroad;
    NFS2_VROAD *vroadData; // Reference using XBID 5
    uint16_t nLanes;
    NFS2_LANE_BLOCK *laneData;
} NFS2_TRKBLOCK;

typedef struct NFS2_SUPERBLOCK {
    uint32_t superBlockSize;
    uint32_t nBlocks;
    uint32_t padding;
    NFS2_TRKBLOCK *trackBlocks;
} NFS2_SUPERBLOCK;


#endif //OPENNFS3_NFS2_DATA_H
