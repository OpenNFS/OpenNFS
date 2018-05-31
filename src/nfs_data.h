//
// Created by Amrik on 26/02/2018.
//

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#ifndef OPENNFS3_NFS_DATA_H
#define OPENNFS3_NFS_DATA_H

#include <cstdint>

namespace NFS3 {
    typedef struct FLOATPT {
        float x, z, y;
    } FLOATPT;

    typedef struct INTPT {
        long x, z, y;
    } INTPT;

    typedef struct NEIGHBORDATA  // info on neighbouring block numbers
    {
        short blk, unknown;
    } NEIGHBORDATA;

    typedef struct POSITIONDATA  // enumerate polygons which lie at center
    {
        short polygon;
        unsigned char nPolygons;
        char unknown;
        short extraNeighbor1, extraNeighbor2;
    } POSITIONDATA;

    typedef struct POLYVROADDATA  // vroad data associated with a polygon
    {
        unsigned char vroadEntry;
        unsigned char flags;
        unsigned char unknown[6];
        unsigned char hs_minmax[4];
        unsigned char hs_orphan[4];
        unsigned char hs_unknown;
    } POLYVROADDATA;

    typedef struct VROADDATA  // vroad vectors
    {
        short xNorm, zNorm, yNorm;
        short xForw, zForw, yForw;
    } VROADDATA;

// WARNING: in the following 2 structures, don't rely on crossindex :
// it's not implemented in NFSHS's REFXOBJ, and due to a bug in T3ED
// refxobj.crossindex doesn't change properly if a polyobj's REFPOLYOBJ
// is deleted !

    typedef struct REFXOBJ // description of a block's XOBJects.
    {
        struct INTPT pt;
        short unknown1;
        short globalno;  // sequence number in all of the track's xobjs
        short unknown2;
        char crossindex; // position in first POLYOBJ chunk (0 if not in first chunk)
        char unknown3;
    } REFXOBJ; // !!! does not list the animated XOBJs

    typedef struct SOUNDSRC {
        struct INTPT refpoint;
        long type;
    } SOUNDSRC;

    typedef struct LIGHTSRC {
        struct INTPT refpoint;
        long type;
    } LIGHTSRC;

    typedef struct TRKBLOCK {
        struct FLOATPT ptCentre;
        struct FLOATPT ptBounding[4];
        long nVertices; // total stored
        long nHiResVert, nLoResVert, nMedResVert; // #poly[...]+#polyobj
        long nVerticesDup, nObjectVert;
        struct FLOATPT *vert;  // the vertices
        long *unknVertices;
        struct NEIGHBORDATA nbdData[0x12C];  // neighboring blocks
        long nStartPos, nPositions;
        long nPolygons, nVRoad, nXobj, nPolyobj, nSoundsrc, nLightsrc;
        struct POSITIONDATA *posData;  // positions along track
        struct POLYVROADDATA *polyData;  // polygon vroad references & flags
        struct VROADDATA *vroadData;   // vroad vectors
        struct REFXOBJ *xobj;
        struct SOUNDSRC *soundsrc;
        struct LIGHTSRC *lightsrc;
        struct FLOATPT hs_ptMin, hs_ptMax;
        long hs_neighbors[8];
    } TRKBLOCK;

    typedef struct POLYGONDATA {
        short vertex[4];
        short texture;
        short unknown1; // only used in road lane polygonblock ?
        unsigned char flags; // 00 normally, 20 at end of row, 10 two-sided
        unsigned char unknown2; // F9
    } POLYGONDATA;

    typedef struct POLYGONDATA *LPPOLYGONDATA;

    typedef struct OBJPOLYBLOCK  // a POLYOBJ chunk
    {
        long n1;        // total number of polygons
        long n2;        // total number of objects including XOBJs
        long nobj;      // not stored in .FRD : number of type 1 objects
        long *types;    // when 1, there is an associated object; else XOBJ
        long *numpoly;  // size of each object (only for type 1 objects)
        LPPOLYGONDATA *poly;    // the polygons themselves
    } OBJPOLYBLOCK;

    typedef struct POLYGONBLOCK {
        long sz[7], szdup[7];
        // 7 blocks == low res / 0 / med. res / 0 / high res / 0 / ??central
        LPPOLYGONDATA poly[7];
        struct OBJPOLYBLOCK obj[4]; // the POLYOBJ chunks
        // if not present, then all objects in the chunk are XOBJs
        // the 1st chunk is described anyway in the TRKBLOCK
    } POLYGONBLOCK;

    typedef struct ANIMDATA {
        struct INTPT pt;
        float costheta, sintheta;
    } ANIMDATA;

    typedef struct XOBJDATA {
        long crosstype; // type 4, or more rarely 3 (animated)
        long crossno;   // obj number from REFXOBJ table in TRKBLOCK
        long unknown;
// this section only for type 4 basic objects
        struct FLOATPT ptRef;
        long unknown2; // in HS, stores the unknown long for type 3 as well
// this section only for type 3 animated objects
        short unknown3[9]; // 6 first are all alike; [6]==[8]=?; [7]=0
        // in HS, only 6 are used ; 6 = expected 4
        char type3, objno;  // type3==3; objno==index among all block's objects?
        short nAnimLength, unknown4;
        struct ANIMDATA *animData;
// common section
        long nVertices;
        struct FLOATPT *vert;  // the vertices
        long *unknVertices;
        long nPolygons;
        struct POLYGONDATA *polyData;  // polygon data
    } XOBJDATA;

    typedef struct XOBJBLOCK {
        long nobj;
        struct XOBJDATA *obj;
    } XOBJBLOCK;

#pragma pack(1)
    typedef struct TEXTUREBLOCK // WARNING: packed but not byte-aligned !!!
    {
        short width, height;
        long unknown1;
        float corners[8]; // 4x planar coordinates == tiling?
        long unknown2;
        char islane;      // 1 if not a real texture (lane), 0 usually
        short texture;    // index in QFS file
    } TEXTUREBLOCK;
#pragma pack()

// the associated COL file

#define XBID_TEXTUREINFO 2
#define XBID_STRUCT3D 8
#define XBID_OBJECT 7
#define XBID_OBJECT2 18
#define XBID_VROAD 15

    typedef struct XBHEAD {
        long size;
        short xbid;
        short nrec;
    } XBHEAD;

    typedef struct COLTEXTUREINFO {
        short texture;  // position in .QFS file
        short unknown1; // zero ?
        short unknown2; // texture offset ?
        short unknown3;
    } COLTEXTUREINFO;

    typedef struct COLVERTEX {
        struct FLOATPT pt; // relative coord
        long unknown;      // like the unknVertices structures in FRD
    } COLVERTEX;

    typedef struct COLPOLYGON {
        short texture;
        char v[4]; // vertices
    } COLPOLYGON;

    typedef struct COLSTRUCT3D {
        long size;
        short nVert, nPoly;
        struct COLVERTEX *vertex;
        struct COLPOLYGON *polygon;
    } COLSTRUCT3D;

    typedef struct COLOBJECT {
        short size;
        char type;     // 1 = basic object, 3 = animated ...
        char struct3D; // reference in previous block
// type 1
        struct INTPT ptRef;
// type 3
        short animLength;
        short unknown;
        struct ANIMDATA *animData; // same structure as in xobjs
    } COLOBJECT;

    typedef struct COLVECTOR {
        signed char x, z, y, unknown;
    } COLVECTOR;

    typedef struct COLVROAD {
        struct INTPT refPt;
        long unknown;  // flags ?
        struct COLVECTOR normal, forward, right;
        long leftWall, rightWall;
    } COLVROAD;

    typedef struct HS_VROADBLOCK { // HS's equivalent to a COLVROAD
        struct FLOATPT refPt;
        struct FLOATPT normal, forward, right;
        float leftWall, rightWall;
        float unknown1[2];
        long unknown2[5];
    } HS_VROADBLOCK;

    typedef struct COLFILE {
        char collID[4]; // 'COLL'
        long version;   // 11
        long fileLength;
        long nBlocks;
        long xbTable[5];
        struct XBHEAD textureHead;
        struct COLTEXTUREINFO *texture;
        struct XBHEAD struct3DHead;
        struct COLSTRUCT3D *struct3D;
        struct XBHEAD objectHead;
        struct COLOBJECT *object;
        struct XBHEAD object2Head;
        struct COLOBJECT *object2;
        struct XBHEAD vroadHead;
        struct COLVROAD *vroad;
        long *hs_extra; // for the extra HS data in COLVROAD
    } COLFILE;
}

namespace NFS2{
    // ---- CORE DATA TYPES ----
    typedef struct VERT {
        int16_t x, z, y;
    } VERT;

    typedef struct VERT_HIGH {
        int32_t x, z, y;
    } VERT_HIGH;

    typedef struct ANIM_POS {
        VERT_HIGH position;
        uint16_t unknown[4];
    } ANIM_POS;

    typedef struct POLYGONDATA {
        uint16_t texture;
        uint16_t otherSideTex;
        uint8_t vertex[4];
    } POLYGONDATA;

    typedef struct VROAD {
        VERT normalVec;
        VERT forwardVec;
    } VROAD;

// ----------------- EXTRA BLOCKS -----------------
    typedef struct EXTRABLOCK_HEADER {
        uint32_t recSize;
        uint16_t XBID;
        uint16_t nRecords;
    } EXTRABLOCK_HEADER;

// Matches number of NP1 polygons in corresponding trackblock
    typedef struct POLY_TYPE {
        // XBID = 5
        uint8_t xblockRef; // Refers to an entry in the XBID=13 extrablock
        uint8_t carBehaviour;
    } POLY_TYPE;

    typedef struct GEOM_BLOCK {
        // XBID = 8
        uint32_t recSize;
        uint16_t nVerts;
        uint16_t nPoly;
        VERT *vertexTable;
        POLYGONDATA *polygonTable;
    } GEOM_BLOCK;

    typedef struct GEOM_REF_BLOCK {
        // XBID = 7, 18
        uint16_t recSize; // If Anim (recType == 3) , recSize == 8 + 20*animation length
        uint8_t recType;
        uint8_t structureRef;
        // Fixed Type (recType == 1)
        VERT_HIGH refCoordinates;
        // Animated Type (recType == 3)
        uint16_t animLength; // num of position records
        uint16_t unknown; // Potentially time between animation steps?
        ANIM_POS *animationData; // Sequence of positions which animation follows
    } GEOM_REF_BLOCK;


// Matches number of full resolution polygons
    typedef struct MEDIAN_BLOCK {
        // XBID = 6
        uint8_t refPoly[8];
    } MEDIAN_BLOCK;


    typedef struct LANE_BLOCK {
        // XBID = 9
        uint8_t vertRef; // Inside B3D structure: 0 to nFullRes + nStickToNext
        uint8_t trackPos; // Position along track inside block (0 to 7)
        uint8_t latPos; // Lateral position, -1 at the end
        uint8_t polyRef; // Inside Full-res B3D structure, 0 to nFullRes
    } LANE_BLOCK;

// ---- COL Specific Extra Blocks ----
    typedef struct TEXTURE_BLOCK {
        // XBID = 2
        uint16_t texNumber; // Texture number in QFS file
        uint16_t alignmentData;
        uint8_t RGB[3]; // Luminosity
        uint8_t RGBlack[3]; // Usually black
    } TEXTURE_BLOCK;

    typedef struct COLLISION_BLOCK {
        // XBID = 15
        VERT_HIGH trackPosition; // Position along track on a single line, either at center or side of road
        int8_t vertVec[3];  // The three vectors are mutually orthogonal, and are normalized so that
        int8_t fwdVec[3];   // each vector's norm is slightly less than 128. Each vector is coded on
        int8_t rightVec[3]; // 3 bytes : its x, z and y components are each signed 8-bit values.
        uint8_t zero;
        uint16_t blockNumber;
        uint16_t unknown;    // The left and right border values indicate the two limits beyond which no car can go. This is the data used for delimitation between the road and scenery
        uint16_t leftBorder; // Formula to find the coordinates of the left-most point of the road is (left-most point) = (reference point) - 2.(left border).(right vector):  there is a factor of 2 between absolute
        uint16_t rightBorder;// 32-bit coordinates and the othe data in the record. Similarly, for the right-most point of the road, (right-most point) = (reference point) + 2.(right border).(right vector).
        uint16_t postCrashPosition; // Lateral position after respawn
        uint32_t unknown2;
    } COLLISION_BLOCK;

// ------------ TRACK BLOCKS ----------------
    typedef struct TRKBLOCK_HEADER {
        uint32_t blockSize;
        uint32_t blockSizeDup;
        uint16_t nExtraBlocks;
        uint16_t unknown;
        uint32_t blockSerial;
        struct VERT_HIGH clippingRect[4];
        uint32_t extraBlockTblOffset;
        uint16_t nStickToNextVerts, nLowResVert, nMedResVert, nHighResVert;
        uint16_t nLowResPoly, nMedResPoly, nHighResPoly; // Possible uint32_t on PC, and uint16_t on PS1
        //uint16_t padding;
    } TRKBLOCK_HEADER;

    typedef struct TRKBLOCK {
        TRKBLOCK_HEADER *header;
        VERT *vertexTable;
        POLYGONDATA *polygonTable;
        POLY_TYPE *polyTypes;
        uint16_t nNeighbours;
        uint16_t *blockNeighbours;
        uint16_t nStructures;
        GEOM_BLOCK *structures;
        uint16_t nStructureReferences;
        GEOM_REF_BLOCK *structureRefData;
        MEDIAN_BLOCK *medianData;
        uint16_t nVroad;
        VROAD *vroadData; // Reference using XBID 5
        uint16_t nLanes;
        LANE_BLOCK *laneData;
    } TRKBLOCK;

    typedef struct SUPERBLOCK {
        uint32_t superBlockSize;
        uint32_t nBlocks;
        uint32_t padding;
        TRKBLOCK *trackBlocks;
    } SUPERBLOCK;

// ---- MASTER TRACK STRUCT ----

    typedef struct TRACK {
        // TRK data
        uint32_t nSuperBlocks;
        uint32_t nBlocks;
        SUPERBLOCK *superblocks;
        // COL data
        uint32_t nTextures;
        TEXTURE_BLOCK *polyToQFStexTable;
        uint32_t nColStructures;
        GEOM_BLOCK *colStructures;
        uint32_t nColStructureReferences;
        GEOM_REF_BLOCK *colStructureRefData;
        uint32_t nCollisionData;
        COLLISION_BLOCK *collisionData;
    } TRACK;

}

#endif //OPENNFS3_NFS_DATA_H
#pragma clang diagnostic pop