#pragma once

#include "../../Common/IRawData.h"
#include "../Common.h"

namespace LibOpenNFS::NFS3 {
#define XBID_TEXTUREINFO 2
#define XBID_STRUCT3D 8
#define XBID_OBJECT 7
#define XBID_OBJECT2 18
#define XBID_VROAD 15

    struct ExtraBlockHeader {
        uint32_t size;
        uint16_t xbid;
        uint16_t nrec;
    };

    struct ColTextureInfo {
        uint16_t id;       // position in .QFS file
        uint16_t unknown1; // zero ?
        uint16_t unknown2; // texture offset ?
        uint16_t unknown3;
    };

    struct ColVertex {
        glm::vec3 pt;     // relative coord
        uint32_t unknown; // like the unknVertices structures in FRD
    };

    struct ColPolygon {
        uint16_t texture;
        char v[4]; // vertices
    };

    struct ColStruct3D {
        uint32_t size;
        uint16_t nVert, nPoly;
        std::vector<ColVertex> vertex;
        std::vector<ColPolygon> polygon;
    };

    struct ColObject {
        uint16_t size;
        uint8_t type;     // 1 = basic object, 3 = animated ...
        uint8_t struct3D; // reference in previous block
        // type 1
        glm::ivec3 ptRef;
        // type 3
        uint16_t animLength;
        uint16_t unknown;
        std::vector<AnimData> animData; // same structure as in xobjs
    };

    struct ColVRoad {
        glm::ivec3 refPt;
        uint32_t unknown; // Unknown data
        glm::i8vec4 normal, forward, right;
        uint32_t leftWall, rightWall;
    };

    class ColFile : IRawData {
    public:
        ColFile() = default;
        static bool Load(const std::string &colPath, ColFile &colFile);
        static void Save(const std::string &colPath, ColFile &colFile);

        char header[4];                      // Header of file 'COLL'
        uint32_t version;                    // Version number 11
        uint32_t fileLength;                 // File length in bytes
        uint32_t nBlocks;                    // Number of Xtra blocks in file
        uint32_t xbTable[5];                 // Offsets of Xtra blocks
        ExtraBlockHeader textureHead = {};   // Record detailing texture table data
        std::vector<ColTextureInfo> texture; // Texture table
        ExtraBlockHeader struct3DHead = {};  // Record detailing struct3D table data
        std::vector<ColStruct3D> struct3D;   // Struct 3D table
        ExtraBlockHeader objectHead = {};    // Record detailing object table data
        std::vector<ColObject> object;       // Object table
        ExtraBlockHeader object2Head = {};   // Record detailing extra object data
        std::vector<ColObject> object2;      // Extra object data
        ExtraBlockHeader vroadHead = {};     // Unknown Record detailing unknown table data
        std::vector<ColVRoad> vroad;         // Unknown table
        uint32_t *hs_extra = nullptr;        // for the extra HS data in ColVRoad

    private:
        bool _SerializeIn(std::ifstream &ifstream) override;
        void _SerializeOut(std::ofstream &ofstream) override;
    };
} // namespace LibOpenNFS::NFS3
