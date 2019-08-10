//
// Created by Amrik on 09/08/2019.
//

#pragma once

#include "stdint.h"

struct VECTOR // hashcode: 0xCB78D6DC (dec: -881273124)
{
    long vx; // size=0, offset=0 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct VECTOR', parent hashcode: 0x00000000 (dec: 0)
    long vy; // size=0, offset=4 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct VECTOR', parent hashcode: 0x00000000 (dec: 0)
    long vz; // size=0, offset=8 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct VECTOR', parent hashcode: 0x00000000 (dec: 0)
    long pad; // size=0, offset=12 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct VECTOR', parent hashcode: 0x00000000 (dec: 0)
};

struct coorddef // hashcode: 0x29F67CAF (dec: 704019631)
{
    int x; // size=0, offset=0 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct coorddef', parent hashcode: 0x00000000 (dec: 0)
    int y; // size=0, offset=4 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct coorddef', parent hashcode: 0x00000000 (dec: 0)
    int z; // size=0, offset=8 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct coorddef', parent hashcode: 0x00000000 (dec: 0)
};

struct COORD16 // hashcode: 0x11C4A969 (dec: 298101097)
{
    short x; // size=0, offset=0 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct COORD16', parent hashcode: 0x00000000 (dec: 0)
    short y; // size=0, offset=2 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct COORD16', parent hashcode: 0x00000000 (dec: 0)
    short z; // size=0, offset=4 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct COORD16', parent hashcode: 0x00000000 (dec: 0)
};

struct Transformer_zUV // hashcode: 0x7FDBB35E (dec: 2145104734)
{
    unsigned char u; // size=0, offset=0 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Transformer_zUV', parent hashcode: 0x00000000 (dec: 0)
    unsigned char v; // size=0, offset=1 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Transformer_zUV', parent hashcode: 0x00000000 (dec: 0)
};

struct Transformer_zFacet // hashcode: 0x90ADBC21 (dec: -1867662303)
{
    short flag; // size=0, offset=0 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Transformer_zFacet', parent hashcode: 0x00000000 (dec: 0)
    unsigned char textureIndex; // size=0, offset=2 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Transformer_zFacet', parent hashcode: 0x00000000 (dec: 0)
    unsigned char vertexId0; // size=0, offset=3 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Transformer_zFacet', parent hashcode: 0x00000000 (dec: 0)
    unsigned char vertexId1; // size=0, offset=4 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Transformer_zFacet', parent hashcode: 0x00000000 (dec: 0)
    unsigned char vertexId2; // size=0, offset=5 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Transformer_zFacet', parent hashcode: 0x00000000 (dec: 0)
    struct Transformer_zUV uv0; // size=2, offset=6 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Transformer_zFacet', parent hashcode: 0x00000000 (dec: 0)
    struct Transformer_zUV uv1; // size=2, offset=8 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Transformer_zFacet', parent hashcode: 0x00000000 (dec: 0)
    struct Transformer_zUV uv2; // size=2, offset=10 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Transformer_zFacet', parent hashcode: 0x00000000 (dec: 0)
};

struct Transformer_zObj // hashcode: 0x9B8A4BF8 (dec: -1685435400)
{
    unsigned short numVertex; // size=0, offset=0 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Transformer_zObj', parent hashcode: 0x00000000 (dec: 0)
    unsigned short numFacet; // size=0, offset=2 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Transformer_zObj', parent hashcode: 0x00000000 (dec: 0)
    struct coorddef translation; // size=12, offset=4 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Transformer_zObj', parent hashcode: 0x00000000 (dec: 0)
    struct COORD16 *vertex; // size=6, offset=16 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Transformer_zObj', parent hashcode: 0x00000000 (dec: 0)
    struct COORD16 *Nvertex; // size=6, offset=20 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Transformer_zObj', parent hashcode: 0x00000000 (dec: 0)
    struct Transformer_zFacet *facet; // size=12, offset=24 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Transformer_zObj', parent hashcode: 0x00000000 (dec: 0)
};

struct Transformer_zOverlay // hashcode: 0xC77675A4 (dec: -948537948)
{
    short offset; // size=0, offset=0 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Transformer_zOverlay', parent hashcode: 0x00000000 (dec: 0)
    unsigned char u; // size=0, offset=2 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Transformer_zOverlay', parent hashcode: 0x00000000 (dec: 0)
    unsigned char v; // size=0, offset=3 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Transformer_zOverlay', parent hashcode: 0x00000000 (dec: 0)
};

struct Transformer_zScene // hashcode: 0xA8EF3F3A (dec: -1460715718)
{
    struct Transformer_zObj *obj[57]; // size=228, offset=0 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Transformer_zScene', parent hashcode: 0x00000000 (dec: 0)
    struct Transformer_zOverlay overlay[90]; // size=360, offset=228 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Transformer_zScene', parent hashcode: 0x00000000 (dec: 0)
};

struct SerializedGroup // hashcode: 0x60C96239 (dec: 1623810617)
{
    int m_type; // size=0, offset=0 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct SerializedGroup', parent hashcode: 0x00000000 (dec: 0)
    int m_length; // size=0, offset=4 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct SerializedGroup', parent hashcode: 0x00000000 (dec: 0)
    int dummy; // size=0, offset=8 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct SerializedGroup', parent hashcode: 0x00000000 (dec: 0)
    int m_num_elements; // size=0, offset=12 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct SerializedGroup', parent hashcode: 0x00000000 (dec: 0)
};

class NFS4PS1Loader {
public:
    static void LoadCar(const std::string &carGeoPath);
};


