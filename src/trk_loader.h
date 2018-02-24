//
// Created by Amrik on 16/01/2018.
//

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#ifndef FCE_TO_OBJ_TRK_LOADER_H
#define FCE_TO_OBJ_TRK_LOADER_H

#include <glm/vec3.hpp>
#include <vector>
#include "boost/filesystem.hpp"
#include <iostream>
#include <fstream>
#include <cstring>
#include "Model.h"

typedef struct FLOATPT
{
    float x,z,y;
} FLOATPT;

typedef struct INTPT
{
    long x,z,y;
} INTPT;

typedef struct NEIGHBORDATA  // info on neighbouring block numbers
{
    short blk,unknown;
} NEIGHBORDATA;

typedef struct POSITIONDATA  // enumerate polygons which lie at center
{
    short polygon;
    unsigned char nPolygons;
    char unknown;
    short extraNeighbor1,extraNeighbor2;
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
    short xNorm,zNorm,yNorm;
    short xForw,zForw,yForw;
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

typedef struct TRKBLOCK
{
    struct FLOATPT ptCentre;
    struct FLOATPT ptBounding[4];
    long nVertices; // total stored
    long nHiResVert,nLoResVert,nMedResVert; // #poly[...]+#polyobj
    long nVerticesDup,nObjectVert;
    struct FLOATPT *vert;  // the vertices
    long *unknVertices;
    struct NEIGHBORDATA nbdData[0x12C];  // neighboring blocks
    long nStartPos,nPositions;
    long nPolygons,nVRoad,nXobj,nPolyobj,nSoundsrc,nLightsrc;
    struct POSITIONDATA *posData;  // positions along track
    struct POLYVROADDATA *polyData;  // polygon vroad references & flags
    struct VROADDATA *vroadData;   // vroad vectors
    struct REFXOBJ *xobj;
    struct SOUNDSRC *soundsrc;
    struct LIGHTSRC *lightsrc;
    struct FLOATPT hs_ptMin,hs_ptMax;
    long hs_neighbors[8];
} TRKBLOCK;

typedef struct POLYGONDATA
{
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
    LPPOLYGONDATA *poly;	// the polygons themselves
} OBJPOLYBLOCK;

typedef struct POLYGONBLOCK
{
    long sz[7],szdup[7];
    // 7 blocks == low res / 0 / med. res / 0 / high res / 0 / ??central
    LPPOLYGONDATA poly[7];
    struct OBJPOLYBLOCK obj[4]; // the POLYOBJ chunks
    // if not present, then all objects in the chunk are XOBJs
    // the 1st chunk is described anyway in the TRKBLOCK
} POLYGONBLOCK;

typedef struct ANIMDATA
{
    struct INTPT pt;
    float costheta,sintheta;
} ANIMDATA;

typedef struct XOBJDATA
{
    long crosstype; // type 4, or more rarely 3 (animated)
    long crossno;   // obj number from REFXOBJ table in TRKBLOCK
    long unknown;
// this section only for type 4 basic objects
    struct FLOATPT ptRef;
    long unknown2; // in HS, stores the unknown long for type 3 as well
// this section only for type 3 animated objects
    short unknown3[9]; // 6 first are all alike; [6]==[8]=?; [7]=0
    // in HS, only 6 are used ; 6 = expected 4
    char type3,objno;  // type3==3; objno==index among all block's objects?
    short nAnimLength,unknown4;
    struct ANIMDATA *animData;
// common section
    long nVertices;
    struct FLOATPT *vert;  // the vertices
    long *unknVertices;
    long nPolygons;
    struct POLYGONDATA *polyData;  // polygon data
} XOBJDATA;

typedef struct XOBJBLOCK
{
    long nobj;
    struct XOBJDATA *obj;
} XOBJBLOCK;

#pragma pack(1)
typedef struct TEXTUREBLOCK // WARNING: packed but not byte-aligned !!!
{
    short width,height;
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
    short nVert,nPoly;
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
    signed char x,z,y,unknown;
} COLVECTOR;

typedef struct COLVROAD {
    struct INTPT refPt;
    long unknown;  // flags ?
    struct COLVECTOR normal,forward,right;
    long leftWall,rightWall;
} COLVROAD;

typedef struct HS_VROADBLOCK { // HS's equivalent to a COLVROAD
    struct FLOATPT refPt;
    struct FLOATPT normal,forward,right;
    float leftWall,rightWall;
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

class Texture {
public:
    unsigned int texture_id, width, height;
    unsigned char* texture_data;

    Texture() = default;
    explicit Texture(unsigned int id, unsigned char *data, unsigned int w, unsigned int h){
        texture_id = id;
        texture_data = data;
        width = w;
        height = h;
    }
};

class trk_loader {
// !!! for arrays : structures are aligned to their largest member
// !!! structure members are aligned on their own size (up to the /Zp parameter)
    // Attributes
        bool bEmpty;
        bool bHSMode;
        char header[28]; /* file header */
        long nBlocks;
        struct TRKBLOCK *trk;
        struct POLYGONBLOCK *poly;
        struct XOBJBLOCK *xobj; // xobj[4*blk+chunk]; global=xobj[4*nblocks]
        long hs_morexobjlen;
        char *hs_morexobj;  // 4N & 4N+1 in HS format (xobj[4N] left empty)
        long nTextures;
        struct TEXTUREBLOCK *texture;
        struct COLFILE col;

    // Implementation
    public:
        explicit trk_loader(const std::string &frd_path);
        virtual ~trk_loader();
        bool LoadFRD(std::string frd_path);
        std::vector<Model> trk_blocks;
        std::vector<Model> getTrackBlocks();
        std::map<short, GLuint> getTextureGLMap();
        std::map<short, Texture> getTextures();

protected:
        std::map<short, GLuint> texture_gl_mappings;
        std::map<short, Texture> textures;
        bool LoadCOL(std::string col_path);
        std::map<short, GLuint> GenTrackTextures(std::map<short, Texture> textures);
};



#endif //FCE_TO_OBJ_TRK_LOADER_H

#pragma clang diagnostic pop