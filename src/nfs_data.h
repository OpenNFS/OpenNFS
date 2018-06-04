//
// Created by Amrik on 26/02/2018.
//

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#ifndef OPENNFS3_NFS_DATA_H
#define OPENNFS3_NFS_DATA_H

#include <cstdint>
#include <map>
#include <vector>
#include <GL/glew.h>

class TrackBlock;

namespace NFS3 {
    typedef struct FLOATPT {
        float x, z, y;
    } FLOATPT;

    typedef struct INTPT {
        uint32_t x, z, y;
    } INTPT;

    typedef struct NEIGHBORDATA  // info on neighbouring block numbers
    {
        uint16_t blk, unknown;
    } NEIGHBORDATA;

    typedef struct POSITIONDATA  // enumerate polygons which lie at center
    {
        uint16_t polygon;
        unsigned char nPolygons;
        char unknown;
        uint16_t extraNeighbor1, extraNeighbor2;
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
        uint16_t xNorm, zNorm, yNorm;
        uint16_t xForw, zForw, yForw;
    } VROADDATA;

// WARNING: in the following 2 structures, don't rely on crossindex :
// it's not implemented in NFSHS's REFXOBJ, and due to a bug in T3ED
// refxobj.crossindex doesn't change properly if a polyobj's REFPOLYOBJ
// is deleted !

    typedef struct REFXOBJ // description of a block's XOBJects.
    {
        struct INTPT pt;
        uint16_t unknown1;
        uint16_t globalno;  // sequence number in all of the track's xobjs
        uint16_t unknown2;
        char crossindex; // position in first POLYOBJ chunk (0 if not in first chunk)
        char unknown3;
    } REFXOBJ; // !!! does not list the animated XOBJs

    typedef struct SOUNDSRC {
        struct INTPT refpoint;
        uint32_t type;
    } SOUNDSRC;

    typedef struct LIGHTSRC {
        struct INTPT refpoint;
        uint32_t type;
    } LIGHTSRC;

    typedef struct TRKBLOCK {
        struct FLOATPT ptCentre;
        struct FLOATPT ptBounding[4];
        uint32_t nVertices; // total stored
        uint32_t nHiResVert, nLoResVert, nMedResVert; // #poly[...]+#polyobj
        uint32_t nVerticesDup, nObjectVert;
        struct FLOATPT *vert;  // the vertices
        uint32_t *unknVertices;
        struct NEIGHBORDATA nbdData[0x12C];  // neighboring blocks
        uint32_t nStartPos, nPositions;
        uint32_t nPolygons, nVRoad, nXobj, nPolyobj, nSoundsrc, nLightsrc;
        struct POSITIONDATA *posData;  // positions auint32_t track
        struct POLYVROADDATA *polyData;  // polygon vroad references & flags
        struct VROADDATA *vroadData;   // vroad vectors
        struct REFXOBJ *xobj;
        struct SOUNDSRC *soundsrc;
        struct LIGHTSRC *lightsrc;
        struct FLOATPT hs_ptMin, hs_ptMax;
        uint32_t hs_neighbors[8];
    } TRKBLOCK;

    typedef struct POLYGONDATA {
        uint16_t vertex[4];
        uint16_t texture;
        uint16_t unknown1; // only used in road lane polygonblock ?
        unsigned char flags; // 00 normally, 20 at end of row, 10 two-sided
        unsigned char unknown2; // F9
    } POLYGONDATA;

    typedef struct POLYGONDATA *LPPOLYGONDATA;

    typedef struct OBJPOLYBLOCK  // a POLYOBJ chunk
    {
        uint32_t n1;        // total number of polygons
        uint32_t n2;        // total number of objects including XOBJs
        uint32_t nobj;      // not stored in .FRD : number of type 1 objects
        uint32_t *types;    // when 1, there is an associated object; else XOBJ
        uint32_t *numpoly;  // size of each object (only for type 1 objects)
        LPPOLYGONDATA *poly;    // the polygons themselves
    } OBJPOLYBLOCK;

    typedef struct POLYGONBLOCK {
        uint32_t sz[7], szdup[7];
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
        uint32_t crosstype; // type 4, or more rarely 3 (animated)
        uint32_t crossno;   // obj number from REFXOBJ table in TRKBLOCK
        uint32_t unknown;
// this section only for type 4 basic objects
        struct FLOATPT ptRef;
        uint32_t unknown2; // in HS, stores the unknown uint32_t for type 3 as well
// this section only for type 3 animated objects
        uint16_t unknown3[9]; // 6 first are all alike; [6]==[8]=?; [7]=0
        // in HS, only 6 are used ; 6 = expected 4
        char type3, objno;  // type3==3; objno==index among all block's objects?
        uint16_t nAnimLength, unknown4;
        struct ANIMDATA *animData;
// common section
        uint32_t nVertices;
        struct FLOATPT *vert;  // the vertices
        uint32_t *unknVertices;
        uint32_t nPolygons;
        struct POLYGONDATA *polyData;  // polygon data
    } XOBJDATA;

    typedef struct XOBJBLOCK {
        uint32_t nobj;
        struct XOBJDATA *obj;
    } XOBJBLOCK;

#pragma pack(1)
    typedef struct TEXTUREBLOCK // WARNING: packed but not byte-aligned !!!
    {
        uint16_t width, height;
        uint32_t unknown1;
        float corners[8]; // 4x planar coordinates == tiling?
        uint32_t unknown2;
        char islane;      // 1 if not a real texture (lane), 0 usually
        uint16_t texture;    // index in QFS file
    } TEXTUREBLOCK;
#pragma pack()

// the associated COL file

#define XBID_TEXTUREINFO 2
#define XBID_STRUCT3D 8
#define XBID_OBJECT 7
#define XBID_OBJECT2 18
#define XBID_VROAD 15

    typedef struct XBHEAD {
        uint32_t size;
        uint16_t xbid;
        uint16_t nrec;
    } XBHEAD;

    typedef struct COLTEXTUREINFO {
        uint16_t texture;  // position in .QFS file
        uint16_t unknown1; // zero ?
        uint16_t unknown2; // texture offset ?
        uint16_t unknown3;
    } COLTEXTUREINFO;

    typedef struct COLVERTEX {
        struct FLOATPT pt; // relative coord
        uint32_t unknown;      // like the unknVertices structures in FRD
    } COLVERTEX;

    typedef struct COLPOLYGON {
        uint16_t texture;
        char v[4]; // vertices
    } COLPOLYGON;

    typedef struct COLSTRUCT3D {
        uint32_t size;
        uint16_t nVert, nPoly;
        struct COLVERTEX *vertex;
        struct COLPOLYGON *polygon;
    } COLSTRUCT3D;

    typedef struct COLOBJECT {
        uint16_t size;
        char type;     // 1 = basic object, 3 = animated ...
        char struct3D; // reference in previous block
// type 1
        struct INTPT ptRef;
// type 3
        uint16_t animLength;
        uint16_t unknown;
        struct ANIMDATA *animData; // same structure as in xobjs
    } COLOBJECT;

    typedef struct COLVECTOR {
        signed char x, z, y, unknown;
    } COLVECTOR;

    typedef struct COLVROAD {
        struct INTPT refPt;
        uint32_t unknown;  // flags ?
        struct COLVECTOR normal, forward, right;
        uint32_t leftWall, rightWall;
    } COLVROAD;

    typedef struct HS_VROADBLOCK { // HS's equivalent to a COLVROAD
        struct FLOATPT refPt;
        struct FLOATPT normal, forward, right;
        float leftWall, rightWall;
        float unknown1[2];
        uint32_t unknown2[5];
    } HS_VROADBLOCK;

    typedef struct COLFILE {
        char collID[4]; // 'COLL'
        uint32_t version;   // 11
        uint32_t fileLength;
        uint32_t nBlocks;
        uint32_t xbTable[5];
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
        uint32_t *hs_extra; // for the extra HS data in COLVROAD
    } COLFILE;


    // ---- NFS3 GL Structures -----
    class Texture {
    public:
        unsigned int texture_id, width, height;
        GLubyte *texture_data;

        Texture() = default;

        explicit Texture(unsigned int id, GLubyte *data, unsigned int w, unsigned int h) {
            texture_id = id;
            texture_data = data;
            width = w;
            height = h;
        }
    };

    // ---- MASTER TRACK STRUCT ----

    typedef struct TRACK {
        // !!! for arrays : structures are aligned to their largest member
        // !!! structure members are aligned on their own size (up to the /Zp parameter)
        // Attributes
        bool bEmpty;
        bool bHSMode;

        uint32_t nBlocks;
        TRKBLOCK *trk;
        POLYGONBLOCK *poly;
        XOBJBLOCK *xobj; // xobj[4*blk+chunk]; global=xobj[4*nblocks]
        uint32_t hs_morexobjlen;
        char *hs_morexobj;  // 4N & 4N+1 in HS format (xobj[4N] left empty)
        uint32_t nTextures;
        TEXTUREBLOCK *texture;
        COLFILE col;
        // GL 3D Render Data
        std::vector<TrackBlock> track_blocks;
        std::map<short, Texture> textures;
        std::map<short, GLuint> texture_gl_mappings;
    } TRACK;
}

namespace NFS2 {
    // ---- CORE DATA TYPES ----
    typedef struct VERT {
        int16_t x, z, y;
    } VERT;

    typedef struct VERT_HIGHP {
        int32_t x, z, y;
    } VERT_HIGHP;

    typedef struct ANIM_POS {
        VERT_HIGHP position;
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
        VERT_HIGHP refCoordinates;
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
        uint8_t trackPos; // Position auint32_t track inside block (0 to 7)
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
        VERT_HIGHP trackPosition; // Position auint32_t track on a single line, either at center or side of road
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
        struct VERT_HIGHP clippingRect[4];
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

namespace Music {
    typedef struct MAPHeader {
        char szID[4];
        uint8_t bUnknown1;
        uint8_t bFirstSection;
        uint8_t bNumSections;
        uint8_t bRecordSize; // ???
        uint8_t Unknown2[3];
        uint8_t bNumRecords;
    } MAPHeader;

    typedef struct MAPSectionDefRecord {
        uint8_t bUnknown;
        uint8_t bMagic;
        uint8_t bNextSection;
    } MAPSectionDefRecord;

    typedef struct MAPSectionDef {
        uint8_t bIndex;
        uint8_t bNumRecords;
        uint8_t szID[2];
        struct MAPSectionDefRecord msdRecords[8];
    } MAPSectionDef;

    struct ASFBlockHeader {
        char szBlockID[4];
        uint32_t dwSize;
    };

    struct ASFChunkHeader {
        uint32_t dwOutSize;
        uint16_t lCurSampleLeft;
        uint16_t lPrevSampleLeft;
        uint16_t lCurSampleRight;
        uint16_t lPrevSampleRight;
    };


}

#endif //OPENNFS3_NFS_DATA_H
#pragma clang diagnostic pop