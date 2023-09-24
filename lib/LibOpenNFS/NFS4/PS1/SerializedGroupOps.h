//
// Created by Amrik on 21/08/2019.
//
#pragma once

struct Group // hashcode: 0xBCD050D8 (dec: -1127198504)
{
    int m_num_elements; // size=0, offset=0 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Group', parent hashcode: 0x00000000
                        // (dec: 0)
};

struct SerializedGroup // hashcode: 0x60C96239 (dec: 1623810617)
{
    int m_type;         // size=0, offset=0 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct SerializedGroup', parent hashcode: 0x00000000
                        // (dec: 0)
    int m_length;       // size=0, offset=4 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct SerializedGroup', parent hashcode: 0x00000000
                        // (dec: 0)
    int dummy;          // size=0, offset=8 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct SerializedGroup', parent hashcode: 0x00000000
                        // (dec: 0)
    int m_num_elements; // size=0, offset=12 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct SerializedGroup', parent hashcode:
                        // 0x00000000 (dec: 0)
};

struct SimpleMem // hashcode: 0xB201899E (dec: -1308522082)
{
    void *heap;      // size=0, offset=0 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct SimpleMem', parent hashcode: 0x00000000 (dec: 0)
    void *freeMem;   // size=0, offset=4 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct SimpleMem', parent hashcode: 0x00000000 (dec: 0)
    int freeMemSize; // size=0, offset=8 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct SimpleMem', parent hashcode: 0x00000000
                     // (dec: 0)
};

struct RelCoord16 // hashcode: 0x58AB2A46 (dec: 1487612486)
{
    short x; // size=0, offset=0 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct RelCoord16', parent hashcode: 0x00000000 (dec: 0)
    short z; // size=0, offset=2 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct RelCoord16', parent hashcode: 0x00000000 (dec: 0)
};

struct Chunk // hashcode: 0xD95D779B (dec: -648185957)
{
    struct RelCoord16 boundPts[4];       // size=16, offset=0 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Chunk', parent hashcode: 0x00000000 (dec: 0)
    struct RelCoord16 chunkboundPts[4];  // size=16, offset=16 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Chunk', parent
                                         // hashcode: 0x00000000 (dec: 0)
    unsigned char quadCounts[6];         // size=6, offset=32 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Chunk', parent hashcode:
                                         // 0x00000000 (dec: 0)
    unsigned char pad[2];                // size=2, offset=38 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Chunk', parent hashcode: 0x00000000 (dec: 0)
    struct Trk_Quad(*renderQuads[4]);    // size=16, offset=40 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Chunk', parent hashcode:
                                         // 0x00000000 (dec: 0)
    struct Group *stripBuf;              // size=4, offset=56 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Chunk', parent hashcode: 0x00000000 (dec: 0)
    struct Group *lorezstripBuf;         // size=4, offset=60 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Chunk', parent hashcode:
                                         // 0x00000000 (dec: 0)
    struct Group *objInstanceBuf;        // size=4, offset=64 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Chunk', parent hashcode:
                                         // 0x00000000 (dec: 0)
    struct Group *objSpecialInstanceBuf; // size=4, offset=68 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Chunk', parent
                                         // hashcode: 0x00000000 (dec: 0)
    struct Group *simSliceBuf;           // size=4, offset=72 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Chunk', parent hashcode: 0x00000000 (dec: 0)
    struct Group *simQuadBuf;            // size=4, offset=76 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Chunk', parent hashcode: 0x00000000 (dec: 0)
    struct Group *simObjBuf;             // size=4, offset=80 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Chunk', parent hashcode: 0x00000000 (dec: 0)
    struct Group *sfxBuf;                // size=4, offset=84 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Chunk', parent hashcode: 0x00000000 (dec: 0)
    struct Group *lineBuf;               // size=4, offset=88 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Chunk', parent hashcode: 0x00000000 (dec: 0)
    struct Group *objVertexBuf;          // size=4, offset=92 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Chunk', parent hashcode:
                                         // 0x00000000 (dec: 0)
    struct Group *objQuadBuf;            // size=4, offset=96 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Chunk', parent hashcode: 0x00000000 (dec: 0)
    struct Group *objQuadInstanceBuf;    // size=4, offset=100 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Chunk', parent hashcode:
                                         // 0x00000000 (dec: 0)
    short firstSimSliceInd;              // size=0, offset=104 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Chunk', parent hashcode:
                                         // 0x00000000 (dec: 0)
    short chunkInd;                      // size=0, offset=106 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Chunk', parent hashcode: 0x00000000 (dec: 0)
    struct Group *vertexBuf;             // size=4, offset=108 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Chunk', parent hashcode: 0x00000000 (dec: 0)
};

struct CVECTOR // hashcode: 0xFAB94110 (dec: -88522480)
{
    unsigned char r;  // size=0, offset=0 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct CVECTOR', parent hashcode: 0x00000000 (dec: 0)
    unsigned char g;  // size=0, offset=1 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct CVECTOR', parent hashcode: 0x00000000 (dec: 0)
    unsigned char b;  // size=0, offset=2 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct CVECTOR', parent hashcode: 0x00000000 (dec: 0)
    unsigned char cd; // size=0, offset=3 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct CVECTOR', parent hashcode: 0x00000000 (dec: 0)
};

struct Trk_NewSimQuad // hashcode: 0x50B351B6 (dec: 1353929142)
{
    unsigned char surface; // size=0, offset=0 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Trk_NewSimQuad', parent hashcode:
                           // 0x00000000 (dec: 0)
};

struct tSaveSurface // hashcode: 0x1C15ABC8 (dec: 471182280)
{
    struct Trk_NewSimQuad *fSimQuad; // size=1, offset=0 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct tSaveSurface', parent
                                     // hashcode: 0x00000000 (dec: 0)
    unsigned char fSurface;          // size=0, offset=4 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct tSaveSurface', parent hashcode:
                                     // 0x00000000 (dec: 0)
};

struct SaveSurface // hashcode: 0x904D8D27 (dec: -1873965785)
{
    short fCount;                // size=0, offset=0 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct SaveSurface', parent hashcode: 0x00000000 (dec: 0)
    short fMaxCount;             // size=0, offset=2 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct SaveSurface', parent hashcode: 0x00000000
                                 // (dec: 0)
    struct tSaveSurface *fStack; // size=8, offset=4 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct SaveSurface', parent hashcode: 0x00000000 (dec: 0)
};

class SerializedGroupOps {};