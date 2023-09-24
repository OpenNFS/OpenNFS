#pragma once

#include "stdint.h"
#include <sstream>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <memory.h>
#include <cstring>
#include <boost/filesystem.hpp>
#include "../../Common/TrackUtils.h"
#include "SerializedGroupOps.h"
#include "../../../../src/Util/Logger.h"
#include "../../../../src/Physics/Car.h"
#include "../../../../src/Util/ImageLoader.h"
#include "../../../../src/Util/Utils.h"
#include "../../../nfs_data.h"

using namespace NFS3_4_DATA::PS1;

// Dumped from IDA, [column 1] % 2 is used to check for part normal presence
static constexpr uint8_t R3DCar_ObjectInfo[57][6] = {
  0x00, 0x49, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x20, 0x02, 0x01, 0x01, 0x00, 0x00, 0x30, 0x00, 0x01, 0x01, 0x00, 0x00, 0xF8, 0x00, 0x00, 0x00, 0x01,
  0x00, 0xF0, 0x08, 0x0A, 0x0A, 0x00, 0x00, 0xE0, 0x00, 0x0C, 0x00, 0x00, 0x00, 0xE0, 0x00, 0x00, 0x0C, 0x00, 0x00, 0xEC, 0x89, 0x0B, 0x0B, 0x00, 0x0B, 0xF0, 0x88, 0x0B, 0x0B,
  0x00, 0x0B, 0xEC, 0x89, 0x0C, 0x0C, 0x00, 0x0C, 0xF0, 0x88, 0x0C, 0x0C, 0x00, 0x0C, 0xE8, 0x00, 0x01, 0x00, 0x00, 0x00, 0xE8, 0x00, 0x00, 0x01, 0x00, 0x00, 0xD4, 0x00, 0x11,
  0x00, 0x00, 0x00, 0xD4, 0x00, 0x11, 0x00, 0x00, 0x00, 0xE1, 0x08, 0x01, 0x00, 0x00, 0x00, 0xE1, 0x08, 0x00, 0x01, 0x00, 0x00, 0xD4, 0x00, 0x12, 0x12, 0x12, 0x00, 0xE2, 0x00,
  0x01, 0x00, 0x00, 0x00, 0xE2, 0x00, 0x00, 0x01, 0x00, 0x00, 0xD4, 0x00, 0x13, 0x13, 0x13, 0x00, 0xE2, 0x18, 0x0F, 0x10, 0x00, 0x00, 0xE2, 0x08, 0x00, 0x01, 0x00, 0x00, 0xD4,
  0x10, 0x14, 0x14, 0x14, 0x00, 0xE2, 0x18, 0x0F, 0x10, 0x00, 0x00, 0xE2, 0x08, 0x00, 0x01, 0x00, 0x00, 0xD4, 0x10, 0x15, 0x15, 0x15, 0x00, 0xE8, 0x08, 0x01, 0x00, 0x00, 0x00,
  0xE8, 0x08, 0x00, 0x01, 0x00, 0x00, 0xD4, 0x00, 0x16, 0x16, 0x16, 0x00, 0xD8, 0x00, 0x01, 0x01, 0x00, 0x00, 0xF4, 0x00, 0x0D, 0x00, 0x00, 0x00, 0xF4, 0x00, 0x0E, 0x00, 0x00,
  0x00, 0xD4, 0x00, 0x11, 0x00, 0x00, 0x00, 0x30, 0x00, 0x02, 0x01, 0x00, 0x00, 0x28, 0x02, 0x03, 0x00, 0x00, 0x03, 0x28, 0x02, 0x03, 0x00, 0x00, 0x03, 0x26, 0x02, 0x04, 0x00,
  0x00, 0x00, 0x24, 0x02, 0x04, 0x00, 0x00, 0x04, 0x24, 0x02, 0x04, 0x00, 0x00, 0x04, 0x00, 0x49, 0x01, 0x00, 0x00, 0x01, 0x00, 0x49, 0x01, 0x00, 0x00, 0x01, 0xF0, 0x80, 0x05,
  0x00, 0x00, 0x05, 0xF0, 0x80, 0x06, 0x00, 0x00, 0x06, 0xE8, 0x89, 0x07, 0x07, 0x00, 0x07, 0xE8, 0x89, 0x08, 0x08, 0x00, 0x08, 0x1F, 0x00, 0x01, 0x01, 0x00, 0x00, 0x1F, 0x00,
  0x01, 0x01, 0x00, 0x00, 0x20, 0x00, 0x01, 0x00, 0x00, 0x00, 0x20, 0x00, 0x01, 0x00, 0x00, 0x00, 0x20, 0x00, 0x09, 0x01, 0x00, 0x00, 0x20, 0x00, 0x09, 0x01, 0x00, 0x00, 0x20,
  0x00, 0x01, 0x00, 0x00, 0x00, 0x20, 0x00, 0x01, 0x00, 0x00, 0x00, 0x20, 0x00, 0x09, 0x01, 0x00, 0x00, 0x20, 0x00, 0x09, 0x01, 0x00, 0x00};

// TODO: Get this confirmed against another vehicle
static constexpr char geoPartNames[57][32] = {"Body Medium",
                                              "Body Low",
                                              "Body Undertray",
                                              "Wheel Wells",
                                              "Wheel Squares (unknown LOD)",
                                              "Wheel Squares (A little bigger)",
                                              "Unknown Invisible??",
                                              "Unknown Invisible??",
                                              "Spoiler Original",
                                              "Spoiler Uprights",
                                              "Spoiler Upgraded",
                                              "Spoiler Upgraded Uprights ",
                                              "Fog Lights and Rear bumper Top",
                                              "Fog Lights and Rear bumper TopR",
                                              "Wing Mirror Attachment Points",
                                              "Wheel Attachment Points",
                                              "Brake Light Quads",
                                              "Unknown Invisible",
                                              "Unknown Rear Light tris",
                                              "Rear Inner Light Quads",
                                              "Rear Inner Light Quads rotated",
                                              "Rear Inner Light Tris",
                                              "Front Light quads",
                                              "Bigger Front Light quads",
                                              "Front Light triangles",
                                              "Rear Main Light Quads",
                                              "Rear Main Light Quads dup",
                                              "Rear Main Light Tris",
                                              "Unknown Invisible",
                                              "Unknown Invisible",
                                              "Front Headlight light Pos",
                                              "Logo and Rear Numberplate",
                                              "Exhaust Tips",
                                              "Low LOD Exhaust Tips",
                                              "Mid Body F/R Triangles",
                                              "Interior Cutoff + Driver Pos",
                                              "Unknown Invisible",
                                              "Unknown Invisible",
                                              "Unknown Invisible",
                                              "Unknown Invisible",
                                              "Unknown Invisible",
                                              "Right Body High",
                                              "Left Body High",
                                              "Right Wing Mirror",
                                              "Left Wing Mirror",
                                              "Front Right Light Bucket",
                                              "Front Left Light bBucket",
                                              "Front Right Wheel",
                                              "Front Left Wheel",
                                              "Unknown Invisible",
                                              "Unknown Invisible",
                                              "Front Right Tire",
                                              "Front Left Tire",
                                              "Unknown Invisible",
                                              "Unknown Invisible",
                                              "Rear Right Wheel",
                                              "Rear Left Wheel"};

struct VECTOR // hashcode: 0xCB78D6DC (dec: -881273124)
{
    long vx;  // size=0, offset=0 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct VECTOR', parent hashcode: 0x00000000 (dec: 0)
    long vy;  // size=0, offset=4 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct VECTOR', parent hashcode: 0x00000000 (dec: 0)
    long vz;  // size=0, offset=8 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct VECTOR', parent hashcode: 0x00000000 (dec: 0)
    long pad; // size=0, offset=12 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct VECTOR', parent hashcode: 0x00000000
    // (dec: 0)
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
    unsigned char u; // size=0, offset=0 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Transformer_zUV', parent hashcode:
    // 0x00000000 (dec: 0)
    unsigned char v; // size=0, offset=1 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Transformer_zUV', parent hashcode:
    // 0x00000000 (dec: 0)
};

struct Transformer_zFacet // hashcode: 0x90ADBC21 (dec: -1867662303)
{
    short flag; // size=0, offset=0 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Transformer_zFacet', parent hashcode:
    // 0x00000000 (dec: 0)
    unsigned char textureIndex; // size=0, offset=2 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Transformer_zFacet',
    // parent hashcode: 0x00000000 (dec: 0)
    unsigned char vertexId0; // size=0, offset=3 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Transformer_zFacet', parent
    // hashcode: 0x00000000 (dec: 0)
    unsigned char vertexId1; // size=0, offset=4 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Transformer_zFacet', parent
    // hashcode: 0x00000000 (dec: 0)
    unsigned char vertexId2; // size=0, offset=5 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Transformer_zFacet', parent
    // hashcode: 0x00000000 (dec: 0)
    struct Transformer_zUV uv0; // size=2, offset=6 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Transformer_zFacet',
    // parent hashcode: 0x00000000 (dec: 0)
    struct Transformer_zUV uv1; // size=2, offset=8 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Transformer_zFacet',
    // parent hashcode: 0x00000000 (dec: 0)
    struct Transformer_zUV uv2; // size=2, offset=10 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Transformer_zFacet',
    // parent hashcode: 0x00000000 (dec: 0)
};

struct Transformer_zObj // hashcode: 0x9B8A4BF8 (dec: -1685435400)
{
    unsigned short numVertex; // size=0, offset=0 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Transformer_zObj', parent
    // hashcode: 0x00000000 (dec: 0)
    unsigned short numFacet; // size=0, offset=2 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Transformer_zObj', parent
    // hashcode: 0x00000000 (dec: 0)
    struct coorddef translation; // size=12, offset=4 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Transformer_zObj',
    // parent hashcode: 0x00000000 (dec: 0)
    struct COORD16 *vertex; // size=6, offset=16 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Transformer_zObj', parent
    // hashcode: 0x00000000 (dec: 0)
    struct COORD16 *Nvertex; // size=6, offset=20 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Transformer_zObj', parent
    // hashcode: 0x00000000 (dec: 0)
    struct Transformer_zFacet *facet; // size=12, offset=24 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct
    // Transformer_zObj', parent hashcode: 0x00000000 (dec: 0)
};

struct Transformer_zOverlay // hashcode: 0xC77675A4 (dec: -948537948)
{
    short offset; // size=0, offset=0 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Transformer_zOverlay', parent
    // hashcode: 0x00000000 (dec: 0)
    unsigned char u; // size=0, offset=2 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Transformer_zOverlay', parent
    // hashcode: 0x00000000 (dec: 0)
    unsigned char v; // size=0, offset=3 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct Transformer_zOverlay', parent
    // hashcode: 0x00000000 (dec: 0)
};

struct Transformer_zScene // hashcode: 0xA8EF3F3A (dec: -1460715718)
{
    struct Transformer_zObj *obj[57]; // size=228, offset=0 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct
    // Transformer_zScene', parent hashcode: 0x00000000 (dec: 0)
    struct Transformer_zOverlay overlay[90]; // size=360, offset=228 //  hashcode: 0x00000000 (dec: 0), parent name: 'struct
    // Transformer_zScene', parent hashcode: 0x00000000 (dec: 0)
};

struct TRACK {
    std::string name;
};

class NFS4PS1 {
public:
    static std::shared_ptr<Car> LoadCar(const std::string &carVivPath);
    static std::shared_ptr<NFS3_4_DATA::PS1::TRACK> LoadTrack(const std::string &trackGrpPath);

private:
    static std::vector<CarModel> LoadGEO(const std::string &geoPath, std::map<unsigned int, Texture> carTextures);
    static void LoadGRP(const std::string &grpPath);
};
