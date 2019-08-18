#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

#pragma once

#include <cstdint>
#include <map>
#include <vector>
#include "Scene/TrackBlock.h"

// ---- NFS2/3/4 GL Structures -----
class Texture {
public:
    unsigned int texture_id, width, height, layer;
    float min_u, min_v, max_u, max_v;

    GLubyte *texture_data;

    Texture() = default;

    explicit Texture(unsigned int id, GLubyte *data, unsigned int w, unsigned int h) {
        texture_id = id;
        texture_data = data;
        width = w;
        height = h;
        min_u = 0.f;
        min_v = 0.f;
        max_u = 0.f;
        max_v = 0.f;
    }
};

/* HEREIN LIES THE HOLY GRAIL OF OLD SCHOOL NEED FOR SPEED MODDING.
 * Every raw data structure from every interesting file in NFS 2, 3, 4 is documented here.
 * Big up to Denis Auroux, Jesper Juul Mortensen, JimDiabolo, Lasse (Nappe1), Hoo, Valery V. Anisimovsky */

namespace SHARED {
    struct CANPT {
        int32_t x, z, y;
        int16_t od1, od2, od3, od4;
    };
}

// Derived from Arushans collated Addict and Jesper-Juul Mortensen notes
// Deprecated in favour of CrpLib
namespace NFS5_DATA {
    struct CRP {
        struct HEADER_INFO {
            uint32_t data; // First 5 bits unknown (0x1A), last 27 bits == Num Parts
            uint32_t getNumParts() {
                return data >> 5;
            }
            uint32_t getUnknown() {
                return data & 0x0000001F;
            }
        };

        struct LENGTH_INFO {
            uint32_t data;
            uint8_t getUnknown() {
                return data >> 24;
            }
            uint32_t getLength() {
                return data >> 8;
            }
        };

        struct HEADER {
            char identifier[4]; // (cars: ' raC'/'Car '; tracks: 'karT'/'Trak')
            HEADER_INFO headerInfo;
            uint32_t nMiscData;
            uint32_t articleTableOffset; // * 16
        };

        // Offset: Header Size, Length: nArticles from Header headerInfo
        struct ARTICLE {
            char identifier[4]; // ('itrA'/'Arti')
            HEADER_INFO headerInfo;
            uint32_t partTableLength; // (* 16)
            uint32_t offset; // Relative from current article offset * 16. Points to a PART_TABLE
        };

        enum PartType {
            MiscPart,
            MaterialPart,
            FshPart,
            BasePart,
            NamePart,
            CullingPart,
            TransformationPart,
            VertexPart,
            NormalPart,
            UVPart,
            TrianglePart,
            EffectPart
        };

        // Offset: Header Size + Article table length, Length: nMiscData * Size of MISC_DATA
        struct MISC_PART {
            uint32_t identifier;
            LENGTH_INFO lengthInfo;
            uint32_t unknown;
            uint32_t offset; // Relative from current MISC_PART offset
        };

        struct MATERIAL_PART {
            uint16_t index;
            char identifier[2]; // ('tm'/'mt')
            LENGTH_INFO lengthInfo;
            uint32_t unknown; // Always seems to be 0x34
            uint32_t offset; // Relative from current MATERIAL_PART offset
        };

        struct FSH_PART {
            uint16_t index;
            char identifier[2]; // ('fs'/'sf')
            LENGTH_INFO lengthInfo;
            uint32_t nFshFiles; // Seems to always be 0x01?
            uint32_t offset; // Relative from current FSH_PART offset
        };

        struct PART_INFO_A {
            uint16_t data;
            // TODO: DO THESE
            // 4 bits
            uint8_t getLOD(){
                return data;
            }
            // 8 bits
            uint16_t getAnimationIndex(){
                return data;
            }
            // 4 bits
            uint8_t getDamageSwitch(){
                return data;
            }
        };

        struct PART_INFO_B {
            uint16_t data;
            // TODO: DO THESE
            // 12 bits
            uint16_t getPartIndex(){
                return data;
            }
            // 4 bits
            uint8_t getLOD(){
                return data;
            }
        };

        struct BASE_PART {
            char identifier[4]; // ('esaB'/'Base')
            LENGTH_INFO lengthInfo; // Length
            uint32_t unknown; // (seems always to be 0x00000000)
            uint32_t offset; // Relative from current BASE_PART offset
        };

        struct NAME_PART {
            char identifier[4]; // ('emaN'/'Name')
            LENGTH_INFO lengthInfo; // Text length
            uint32_t unknown; // (seems always to be 0x00000000)
            uint32_t offset; // Relative from current NAME_PART offset
        };

        struct CULLING_PART {
            PART_INFO_A partInfo;
            char identifier[2]; // ('n$'/'$n')
            LENGTH_INFO lengthInfo;
            uint32_t nCullingParts; // (seems always to be 0x00000000)
            uint32_t offset; // Relative from current CULLING_PART offset
        };

        struct TRANSFORMATION_PART {
            PART_INFO_A partInfo;
            char identifier[2]; // ('rt'/'tr')
            LENGTH_INFO lengthInfo;
            uint32_t nTransformMatrices; // Always 1
            uint32_t offset; // Relative from current TRANSFORMATION_PART offset
        };

        struct VERTEX_PART {
            PART_INFO_A partInfo;
            char identifier[2]; // ('tv'/'vt')
            LENGTH_INFO lengthInfo;
            uint32_t nVertices;
            uint32_t offset; // Relative from current VERTEX_PART offset
        };

        struct NORMAL_PART {
            PART_INFO_A partInfo;
            char identifier[2]; // (('mn'/'nm')
            LENGTH_INFO lengthInfo;
            uint32_t nNormals;
            uint32_t offset; // Relative from current NORMAL_PART offset
        };

        struct UV_PART {
            PART_INFO_A partInfo;
            char identifier[2]; // ('vu'/'uv')
            LENGTH_INFO lengthInfo;
            uint32_t nUVS;
            uint32_t offset; // Relative from current UV_PART offset
        };

        struct TRIANGLE_PART {
            PART_INFO_B partInfo;
            char identifier[2]; // ('rp'/'pr')
            LENGTH_INFO lengthInfo;
            uint32_t nIndices;
            uint32_t offset; // Relative from current TRIANGLE_PART offset
        };

        struct EFFECT_PART {
            PART_INFO_A partInfo;
            char identifier[2]; // ('fe'/'ef')
            LENGTH_INFO lengthInfo;
            uint32_t nEffects;
            uint32_t offset; // Relative from current EFFECT_PART offset
        };

        union GENERIC_PART {
            // Misc Parts
            MISC_PART miscPart;
            MATERIAL_PART materialPart;
            FSH_PART fshPart;
            // Part table parts
            BASE_PART basePart;
            NAME_PART namePart;
            CULLING_PART cullingPart;
            TRANSFORMATION_PART transformationPart;
            VERTEX_PART vertexPart;
            NORMAL_PART normalPart;
            UV_PART uvPart;
            TRIANGLE_PART trianglePart;
            EFFECT_PART effectPart;
            PartType getPartType(){
                if((strncmp(materialPart.identifier, "tm", 2) == 0) || strncmp(materialPart.identifier, "mt", 2) == 0){
                    return PartType::MaterialPart;
                } else if ((strncmp(fshPart.identifier, "fs", 2) == 0) || strncmp(fshPart.identifier, "sf", 2) == 0){
                    return PartType::FshPart;
                } else if((strncmp(basePart.identifier, "esaB", 4) == 0) || strncmp(basePart.identifier, "Base", 4) == 0){
                    return PartType::BasePart;
                } else if ((strncmp(namePart.identifier, "emaN", 4) == 0) || strncmp(namePart.identifier, "Name", 4) == 0){
                    return PartType::NamePart;
                } else if ((strncmp(cullingPart.identifier, "n$", 2) == 0) || strncmp(cullingPart.identifier, "$n", 2) == 0){
                    return PartType::CullingPart;
                } else if ((strncmp(transformationPart.identifier, "rt", 2) == 0) || strncmp(transformationPart.identifier, "tr", 2) == 0){
                    return PartType::TransformationPart;
                } else if ((strncmp(vertexPart.identifier, "tv", 2) == 0) || strncmp(vertexPart.identifier, "vt", 2) == 0){
                    return PartType::VertexPart;
                } else if ((strncmp(normalPart.identifier, "mn", 2) == 0) || strncmp(normalPart.identifier, "nm", 2) == 0){
                    return PartType::NormalPart;
                } else if ((strncmp(uvPart.identifier, "vu", 2) == 0) || strncmp(uvPart.identifier, "uv", 2) == 0){
                    return PartType::UVPart;
                } else if ((strncmp(trianglePart.identifier, "rp", 2) == 0) || strncmp(trianglePart.identifier, "pr", 2) == 0){
                    return PartType::TrianglePart;
                } else if ((strncmp(effectPart.identifier, "fe", 2) == 0) || strncmp(effectPart.identifier, "ef", 2) == 0){
                    return PartType::EffectPart;
                } else {
                    return PartType::MiscPart;
                }
            }
        };

        // ONFS Helper structure
        // TODO: It's probably time to get OOP with this...
        struct ARTICLE_DATA {
            explicit ARTICLE_DATA(uint32_t articleIdx) : index(articleIdx) {};
            uint32_t index;
            // Raw CRP Part Data
            std::vector<BASE_PART> baseParts;
            std::vector<NAME_PART> nameParts;
            std::vector<CULLING_PART> cullingParts;
            std::vector<TRANSFORMATION_PART> transformationParts;
            std::vector<VERTEX_PART> vertexParts;
            std::vector<NORMAL_PART> normalParts;
            std::vector<UV_PART> uvParts;
            std::vector<TRIANGLE_PART> triangleParts;
            std::vector<EFFECT_PART> effectParts;
            std::vector<MISC_PART> miscParts;
            // ONFS Data pointed to by raw CRP Parts
            std::vector<std::vector<glm::vec3>> vertPartTableData;
        };

        struct VERTEX {
            float x, y, z;
            float unknown; // Always 0x3F80000
        };

        struct TRIANGLE_DATA {
            uint16_t fillMode;
            uint16_t transformInfo;
            uint16_t material;
            uint16_t unknown;
            glm::vec4 boundingSphere;
            glm::vec4 unknown1;
            uint32_t nInfoRows;
            uint32_t nIndexRows;
        };
    };
};

namespace NFS3_4_DATA {
    struct FLOATPT {
        float x, z, y;
    };

    struct INTPT {
        int32_t x, z, y;
    };

    struct NEIGHBORDATA  // info on neighbouring block numbers
    {
        int16_t blk, unknown;
    };

    struct POSITIONDATA  // enumerate polygons which lie at center
    {
        uint16_t polygon;
        unsigned char nPolygons;
        char unknown;
        int16_t extraNeighbor1, extraNeighbor2;
    };

    struct POLYVROADDATA  // vroad data associated with a polygon
    {
        unsigned char vroadEntry;
        unsigned char flags;
        unsigned char unknown[6];
        unsigned char hs_minmax[4];
        unsigned char hs_orphan[4];
        unsigned char hs_unknown;
    };

    struct VROADDATA  // vroad vectors
    {
        uint16_t xNorm, zNorm, yNorm;
        uint16_t xForw, zForw, yForw;
    };

// WARNING: in the following 2 structures, don't rely on crossindex :
// it's not implemented in NFSHS's REFXOBJ, and due to a bug in T3ED
// refxobj.crossindex doesn't change properly if a polyobj's REFPOLYOBJ
// is deleted !

    struct REFXOBJ // description of a block's XOBJects.
    {
        INTPT pt;
        uint16_t unknown1;
        uint16_t globalno;  // sequence number in all of the track's xobjs
        uint16_t unknown2;
        char crossindex; // position in first POLYOBJ chunk (0 if not in first chunk)
        char unknown3;
    }; // !!! does not list the animated XOBJs

    struct SOUNDSRC {
        INTPT refpoint;
        uint32_t type;
    };

    struct LIGHTSRC {
        INTPT refpoint;
        uint32_t type;
    };

    struct TRKBLOCK {
        FLOATPT ptCentre;
        FLOATPT ptBounding[4];
        int32_t nVertices; // total stored
        uint32_t nHiResVert, nLoResVert, nMedResVert; // #poly[...]+#polyobj
        uint32_t nVerticesDup, nObjectVert;
        FLOATPT *vert;  // the vertices
        uint32_t *unknVertices;
        NEIGHBORDATA nbdData[0x12C];  // neighboring blocks
        uint32_t nStartPos, nPositions;
        uint32_t nPolygons, nVRoad, nXobj, nPolyobj, nSoundsrc, nLightsrc;
        POSITIONDATA *posData;  // positions auint32_t track
        POLYVROADDATA *polyData;  // polygon vroad references & flags
        VROADDATA *vroadData;   // vroad vectors
        REFXOBJ *xobj;
        SOUNDSRC *soundsrc;
        LIGHTSRC *lightsrc;
        FLOATPT hs_ptMin, hs_ptMax;
        uint32_t hs_neighbors[8];
    };

    struct POLYGONDATA {
        uint16_t vertex[4];
        uint16_t texture;
        uint16_t hs_texflags; // only used in road lane polygonblock ?
        unsigned char flags; // 00 normally, 20 at end of row, 10 two-sided (HS  // used for animated textures //AnimInfo (Length : Period AS LSB 3:HSB 5))
        unsigned char unknown2; // F9
    };

    typedef struct POLYGONDATA *LPPOLYGONDATA;

    struct OBJPOLYBLOCK  // a POLYOBJ chunk
    {
        uint32_t n1;        // total number of polygons
        uint32_t n2;        // total number of objects including XOBJs
        uint32_t nobj;      // not stored in .FRD : number of type 1 objects
        uint32_t *types;    // when 1, there is an associated object; else XOBJ
        uint32_t *numpoly;  // size of each object (only for type 1 objects)
        LPPOLYGONDATA *poly;    // the polygons themselves
    };

    struct POLYGONBLOCK {
        uint32_t sz[7], szdup[7];
        // 7 blocks == low res / 0 / med. res / 0 / high res / 0 / ??central
        LPPOLYGONDATA poly[7];
        OBJPOLYBLOCK obj[4]; // the POLYOBJ chunks
        // if not present, then all objects in the chunk are XOBJs
        // the 1st chunk is described anyway in the TRKBLOCK
    };

    struct ANIMDATA {
        INTPT pt;
        int16_t od1, od2, od3, od4;
    };

    struct XOBJDATA {
        uint32_t crosstype; // type 4, or more rarely 3 (animated)
        uint32_t crossno;   // obj number from REFXOBJ table in TRKBLOCK
        uint32_t unknown;
// this section only for type 4 basic objects
        FLOATPT ptRef;
        uint32_t AnimMemory; // in HS, stores the unknown uint32_t for type 3 as well
// this section only for type 3 animated objects
        uint16_t unknown3[9]; // 6 first are all alike; [6]==[8]=?; [7]=0
        // in HS, only 6 are used ; 6 = expected 4
        char type3, objno;  // type3==3; objno==index among all block's objects?
        uint16_t nAnimLength, AnimDelay; //JimDiabolo : The bigger the AnimDelay, that slower is the movement
        ANIMDATA *animData;
// common section
        uint32_t nVertices;
        FLOATPT *vert;  // the vertices
        uint32_t *unknVertices;
        uint32_t nPolygons;
        POLYGONDATA *polyData;  // polygon data
    };

    struct XOBJBLOCK {
        uint32_t nobj;
        XOBJDATA *obj;
    };

#pragma pack(1)
    struct TEXTUREBLOCK // WARNING: packed but not byte-aligned !!!
    {
        uint16_t width, height;
        uint32_t unknown1;
        float corners[8]; // 4x planar coordinates == tiling?
        uint32_t unknown2;
        char islane;      // 1 if not a real texture (lane), 0 usually
        uint16_t texture;    // index in QFS file
    };
#pragma pack()

// the associated COL file

#define XBID_TEXTUREINFO 2
#define XBID_STRUCT3D 8
#define XBID_OBJECT 7
#define XBID_OBJECT2 18
#define XBID_VROAD 15

    struct XBHEAD {
        uint32_t size;
        uint16_t xbid;
        uint16_t nrec;
    };

    struct COLTEXTUREINFO {
        uint16_t texture;  // position in .QFS file
        uint16_t unknown1; // zero ?
        uint16_t unknown2; // texture offset ?
        uint16_t unknown3;
    };

    struct COLVERTEX {
        FLOATPT pt; // relative coord
        uint32_t unknown;      // like the unknVertices structures in FRD
    };

    struct COLPOLYGON {
        uint16_t texture;
        char v[4]; // vertices
    };

    struct COLSTRUCT3D {
        uint32_t size;
        uint16_t nVert, nPoly;
        COLVERTEX *vertex;
        COLPOLYGON *polygon;
    };

    struct COLOBJECT {
        uint16_t size;
        char type;     // 1 = basic object, 3 = animated ...
        char struct3D; // reference in previous block
// type 1
        INTPT ptRef;
// type 3
        uint16_t animLength;
        uint16_t unknown;
        ANIMDATA *animData; // same structure as in xobjs
    };

    struct COLVECTOR {
        signed char x, z, y, unknown;
    };

    struct COLVROAD {
        INTPT refPt;
        uint32_t unknown;  // Unknown data
        COLVECTOR normal, forward, right;
        uint32_t leftWall, rightWall;
    };

    struct HS_VROADBLOCK { // HS's equivalent to a COLVROAD
        FLOATPT refPt;
        FLOATPT normal, forward, right;
        float leftWall, rightWall;
        float unknown1[2];
        uint32_t unknown2[5];
    };

    struct COLFILE {
        char collID[4]; // Header of file 'COLL'
        uint32_t version;   // Version number 11
        uint32_t fileLength; // File length in bytes
        uint32_t nBlocks; // Number of Xtra blocks in file
        uint32_t xbTable[5]; // Offsets of Xtra blocks
        XBHEAD textureHead; // Record detailing texture table data
        COLTEXTUREINFO *texture; // Texture table
        XBHEAD struct3DHead; // Record detailing struct3D table data
        COLSTRUCT3D *struct3D; // Struct 3D table
        XBHEAD objectHead; // Record detailing object table data
        COLOBJECT *object; // Object table
        XBHEAD object2Head; // Record detailing extra object data
        COLOBJECT *object2; // Extra object data
        XBHEAD vroadHead; // Unknown Record detailing unknown table data
        COLVROAD *vroad;  // Unknown table
        uint32_t *hs_extra; // for the extra HS data in COLVROAD
    };

    // ---- MASTER TRACK STRUCT ----
    struct TRACK {
        // !!! for arrays : structures are aligned to their largest member
        // !!! structure members are aligned on their own size (up to the /Zp parameter)
        // Attributes
        bool bEmpty;
        bool bHSMode;
        std::string name;
        uint32_t nBlocks;
        TRKBLOCK *trk;
        POLYGONBLOCK *poly;
        XOBJBLOCK *xobj; // xobj[4*blk+chunk]; global=xobj[4*nblocks]
        uint32_t hs_morexobjlen;
        char *hs_morexobj;  // 4N & 4N+1 in HS format (xobj[4N] left empty)
        uint32_t nTextures;
        TEXTUREBLOCK *texture;
        COLFILE col;
        std::vector<SHARED::CANPT> cameraAnimation;
        // GL 3D Render Data
        std::vector<TrackBlock> track_blocks;
        std::vector<Entity> global_objects;
        std::map<unsigned int, Texture> textures;
        GLuint textureArrayID;
        glm::vec3 sky_top_colour, sky_bottom_colour;
    };

    struct FCE {
        struct TVECTOR {
            float x, y, z;
        };

        struct TRIANGLE {
            uint32_t texPage;
            uint32_t vertex[3]; // Local indexes, add part first Vert index from "partFirstVertIndices"
            uint16_t padding[6]; // 00FF
            uint32_t polygonFlags;
            float uvTable[6]; // U1 U2 U3, V1 V2 V3
        };

        struct NFS4 {
            struct COLOUR {
                uint8_t H, S, B, T;
            };

            // Valid values for components:
            //    K : "H" (Headlights); "T" (Taillights); "B" (Brakelight); "R" (Reverse light); "P" (Direction indicator); "S" (Siren);
            //    C : "W" (White); "R" (Red); "B" (Blue); "O" (Orange); "Y" (Yellow)
            //    B : "Y" (Yes); "N" (No)
            //    F : "O" (Flashing at moment 1); "E" (Flashing at moment 2); "N" (No flashing)
            //    I : Number between 0 and 9 with 0 being broken (normal max 5)
            //   Next only used with flashing lights:
            //    T : Number between 1 and 9 with 9 being longest time and 0 being constant (normal max 5)
            //    D : Number between 0 and 9 with 9 being longest delay and 0 no delay (normal max 2)
            struct DUMMY {
                char data[64];
                //char kind, colour, breakable, flashing, intensity, time, delay;
            };

            struct HEADER {
                uint32_t header; // Value always seems to be 14 10 10 00
                uint32_t unknown;
                uint32_t nTriangles;
                uint32_t nVertices;
                uint32_t nArts;

                uint32_t vertTblOffset;
                uint32_t normTblOffset;
                uint32_t triTblOffset;

                uint32_t tempStoreOffsets[3]; // -- ALL offset from 0x2038
                uint32_t undamagedVertsOffset;
                uint32_t undamagedNormsOffset;
                uint32_t damagedVertsOffset;
                uint32_t damagedNormsOffset;
                uint32_t unknownAreaOffset;
                uint32_t driverMovementOffset;
                uint32_t unknownOffsets[2];

                float modelHalfSize[3]; // X, Y, Z

                uint32_t nDummies; // 0..16
                FLOATPT dummyCoords[16];

                uint32_t nParts;
                FLOATPT partCoords[64];

                uint32_t partFirstVertIndices[64];
                uint32_t partNumVertices[64];
                uint32_t partFirstTriIndices[64];
                uint32_t partNumTriangles[64];

                uint32_t nColours;
                COLOUR primaryColours[16];
                COLOUR interiorColours[16];
                COLOUR secondaryColours[16];
                COLOUR driverHairColours[16];

                uint8_t unknownTable[260]; // Probably part related, with 4 byte table header?

                DUMMY dummyObjectInfo[16];

                char partNames[64][64];

                uint8_t unknownTable2[528];
            };
        };

        struct NFS3 {
            struct COLOUR {
                uint32_t H, S, B, T;
            };

            struct HEADER {
                uint32_t unknown;

                uint32_t nTriangles;
                uint32_t nVertices;
                uint32_t nArts;

                uint32_t vertTblOffset;
                uint32_t normTblOffset;
                uint32_t triTblOffset;
                uint32_t reserve1Offset;
                uint32_t reserve2Offset;
                uint32_t reserve3Offset;

                FLOATPT modelHalfSize;

                uint32_t nDummies;
                FLOATPT dummyCoords[16];

                uint32_t nParts;
                FLOATPT partCoords[64];

                uint32_t partFirstVertIndices[64];
                uint32_t partNumVertices[64];
                uint32_t partFirstTriIndices[64];
                uint32_t partNumTriangles[64];

                uint32_t nPriColours;
                COLOUR primaryColours[16];
                uint32_t nSecColours;
                COLOUR secondaryColours[16];

                char dummyNames[16][64];
                char partNames[64][64];

                uint32_t unknownTable[64];
            };
        };
    };

    struct FEDATA {
        struct NFS4 {
            static const int COLOUR_TABLE_OFFSET = 0x043C;
            static const int MENU_NAME_FILEPOS_OFFSET = 0x03C8;
        };

        struct NFS3 {
            static const int COLOUR_TABLE_OFFSET = 0xA7;
            static const int MENU_NAME_FILEPOS_OFFSET = 0x37;
        };

    };
#pragma pack(1)
    struct FFN {
        struct HEADER {
            char fntfChk[4];
            uint32_t fileSize;
            uint16_t version;
            uint16_t numChars;
            uint32_t unknown;
            uint16_t zeroPad;
            uint16_t unknown2;
            uint64_t num20Check;
            uint32_t fontMapOffset;
        };

        struct CHAR_TABLE_ENTRY {
            uint16_t asciiCode;
            uint8_t width;
            uint8_t height;
            uint8_t horizontalScroll;
            uint8_t unknown[2];
            uint8_t zeroPad;
            uint8_t rightPadding;
            uint8_t leftPadding;
            uint8_t topPadding;
        };
    };
#pragma pack()

    struct PS1 {
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

    };
};

namespace NFS2_DATA {
    // ---- CORE DATA TYPES ----
    struct VERT_HIGHP {
        int32_t x, z, y;
    };

    struct ANIM_POS {
        VERT_HIGHP position;
        int16_t unknown[4];
    };

    // ----------------- EXTRA BLOCKS -----------------
    struct EXTRABLOCK_HEADER {
        uint32_t recSize;
        uint16_t XBID;
        uint16_t nRecords;
    };

    // Matches number of NP1 polygons in corresponding trackblock
    struct POLY_TYPE {
        // XBID = 5
        uint8_t xblockRef; // Refers to an entry in the XBID=13 extrablock
        uint8_t carBehaviour;
    };

    struct GEOM_REF_BLOCK {
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
    };

    // Matches number of full resolution polygons
    struct MEDIAN_BLOCK {
        // XBID = 6
        uint8_t refPoly[8];
    };

    struct LANE_BLOCK {
        // XBID = 9
        uint8_t vertRef; // Inside B3D structure: 0 to nFullRes + nStickToNext
        uint8_t trackPos; // Position auint32_t track inside block (0 to 7)
        uint8_t latPos; // Lateral position, -1 at the end
        uint8_t polyRef; // Inside Full-res B3D structure, 0 to nFullRes
    };

    // ---- COL Specific Extra Blocks ----
    struct TEXTURE_BLOCK {
        // XBID = 2
        uint16_t texNumber; // Texture number in QFS file
        uint16_t alignmentData;
        uint8_t RGB[3]; // Luminosity
        uint8_t RGBlack[3]; // Usually black
    };

    struct COLLISION_BLOCK {
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
    };

    // ------------ TRACK BLOCKS ----------------
    struct TRKBLOCK_HEADER {
        uint32_t blockSize;
        uint32_t blockSizeDup;
        uint16_t nExtraBlocks;
        uint16_t unknown;
        uint32_t blockSerial;
        struct VERT_HIGHP clippingRect[4];
        uint32_t extraBlockTblOffset;
        uint16_t nStickToNextVerts, nLowResVert, nMedResVert, nHighResVert;
        uint16_t nLowResPoly, nMedResPoly, nHighResPoly;
        uint16_t unknownPad[3];
    };

    struct PC {
        // ---- CORE DATA TYPES ----
        struct POLYGONDATA {
            int16_t texture;
            int16_t otherSideTex;
            uint8_t vertex[4];
        };

        struct VERT {
            int16_t x, z, y;
        };

        // (Unfortunately, structs below contain the above two modified structs, so must be duplicated across PS1/PC)
        struct VROAD {
            VERT normalVec;
            VERT forwardVec;
        };

        // ---- EXTRA BLOCKS ----
        struct GEOM_BLOCK {
            // XBID = 8
            uint32_t recSize;
            uint16_t nVerts;
            uint16_t nPoly;
            VERT *vertexTable;
            POLYGONDATA *polygonTable;
        };

        // ---- TRACK BLOCKS ----
        struct TRKBLOCK {
            TRKBLOCK_HEADER *header;
            VERT *vertexTable;
            POLYGONDATA *polygonTable;
            POLY_TYPE *polyTypes;
            uint16_t nNeighbours;
            uint16_t *blockNeighbours;
            uint16_t nStructures;
            GEOM_BLOCK *structures;

            uint16_t nStructureReferences;
            std::vector<GEOM_REF_BLOCK> structureRefData;

            MEDIAN_BLOCK *medianData;
            uint16_t nVroad;
            VROAD *vroadData; // Reference using XBID 5
            uint16_t nLanes;
            LANE_BLOCK *laneData;
        };

        struct SUPERBLOCK {
            uint32_t superBlockSize;
            uint32_t nBlocks;
            uint32_t padding;
            TRKBLOCK *trackBlocks;
        };

        // ---- MASTER TRACK STRUCT ----
        struct TRACK {
            std::string name;
            // TRK data
            uint32_t nSuperBlocks;
            uint32_t nBlocks;
            SUPERBLOCK *superblocks;
            VERT_HIGHP *blockReferenceCoords;
            // COL data
            uint32_t nTextures;
            TEXTURE_BLOCK *polyToQFStexTable;
            uint32_t nColStructures;
            GEOM_BLOCK *colStructures;
            uint32_t nColStructureReferences;
            std::vector<GEOM_REF_BLOCK> colStructureRefData;
            uint32_t nCollisionData;
            COLLISION_BLOCK *collisionData;
            std::vector<SHARED::CANPT> cameraAnimation;
            // GL 3D Render Data
            std::vector<Entity> global_objects;
            std::vector<TrackBlock> track_blocks;
            std::map<unsigned int, Texture> textures;
            GLuint textureArrayID;
        };

        struct GEO {
#pragma pack(push, 2)
            struct HEADER {
                uint32_t padding; // Possible value: 0x00, 0x01, 0x02
                uint32_t unknown[32]; // useless list with values, which increase by 0x4 (maybe global offset list, which is needed for calculating the position of the blocks)
                uint64_t unknown2; //  always 0x00
            };


            struct BLOCK_HEADER {
                uint32_t nVerts; // If nVert = 0x00, jump sizeof(GEO_BLOCK_HEADER) forwards, if odd, add 1
                uint32_t nPolygons;
                int32_t position[3]; // Absolute XYZ of the block
                uint16_t unknown;  // ? similar to the value in the list above
                uint16_t unknown1; // ? similar to the value in the list above
                uint16_t unknown2; // ? similar to the value in the list above
                uint16_t unknown3; // ? similar to the value in the list above
                uint64_t pad0; // always 0x00
                uint64_t pad1; // always 0x01
                uint64_t pad2; // always 0x01
            };
#pragma pack(pop)

            // Maybe this is a platform specific VERT HIGH P scenario?
            struct BLOCK_3D {
                int16_t x;
                int16_t y;
                int16_t z;
            };

            struct POLY_3D {
                uint32_t texMapType;
                uint8_t vertex[4];
                char texName[4];
            };
        };
    };

    struct PS1 {
        struct POLYGONDATA {
            uint8_t texture;
            uint8_t otherSideTex;
            uint8_t vertex[4];
        };

        struct VERT {
            int16_t x, z, y, w;
        };

        struct VROAD {
            VERT normalVec;
            VERT forwardVec;
        };

        // ---- EXTRA BLOCKS ----
        struct GEOM_BLOCK {
            // XBID = 8
            uint32_t recSize;
            uint16_t nVerts;
            uint16_t nPoly;
            VERT *vertexTable;
            POLYGONDATA *polygonTable;
        };

        // ---- TRACK BLOCKS ----
        struct TRKBLOCK {
            TRKBLOCK_HEADER *header;
            VERT *vertexTable;
            POLYGONDATA *polygonTable;
            POLY_TYPE *polyTypes;
            uint16_t nNeighbours;
            uint16_t *blockNeighbours;
            uint16_t nStructures;
            GEOM_BLOCK *structures;

            uint16_t nStructureReferences;
            std::vector<GEOM_REF_BLOCK> structureRefData;

            uint16_t nUnknownVerts;
            VERT *unknownVerts;

            MEDIAN_BLOCK *medianData;
            uint16_t nVroad;
            VROAD *vroadData; // Reference using XBID 5
            uint16_t nLanes;
            LANE_BLOCK *laneData;
        };

        struct SUPERBLOCK {
            uint32_t superBlockSize;
            uint32_t nBlocks;
            uint32_t padding;
            TRKBLOCK *trackBlocks;
        };

        // ---- MASTER TRACK STRUCT ----
        struct TRACK {
            std::string name;
            // TRK data
            uint32_t nSuperBlocks;
            uint32_t nBlocks;
            SUPERBLOCK *superblocks;
            VERT_HIGHP *blockReferenceCoords;
            // COL data
            uint32_t nTextures;
            TEXTURE_BLOCK *polyToQFStexTable;
            uint32_t nColStructures;
            GEOM_BLOCK *colStructures;

            uint32_t nColStructureReferences;
            std::vector<GEOM_REF_BLOCK> colStructureRefData;

            uint32_t nCollisionData;
            COLLISION_BLOCK *collisionData;

            std::vector<SHARED::CANPT> cameraAnimation;
            // GL 3D Render Data
            std::vector<Entity> global_objects;
            std::vector<TrackBlock> track_blocks;
            std::map<unsigned int, Texture> textures;
            GLuint textureArrayID;
        };

        struct PSH {
            struct HEADER {
                char header[4]; //  "SHPP"
                uint32_t length; // Inclusive Length of the PSH file
                uint32_t nDirectories; // Number of directory entries
                char chk[4]; // "GIMX"
            };

            struct DIR_ENTRY {
                char imageName[4];
                uint32_t imageOffset; // Offset to start of the image, len implied by difference between offsets to next
            };

            struct IMAGE_HEADER {
                uint8_t imageType; // Image type: Observed values are 0x40, 0x42, 0x43, and 0xC0 The bottom 2 bits of the image type byte specify the bit depth of the image: 0 - 4-bit indexed colour 2 - 16-bit direct colour 3 - 24-bit direct colour
                uint8_t unknown[3];
                uint16_t width;
                uint16_t height;
                uint16_t unknown2[4];
            };

            struct PALETTE_HEADER {
                uint32_t unknown;
                uint16_t paletteWidth; // Always 16
                uint16_t paletteHeight; // Always 1
                uint16_t nPaletteEntries; // Always 16
                uint16_t unknown2[3]; // [0] always 0 [1] always 0 [2] often 240, sometimes 0
            };
        };

        struct GEO {
#pragma pack(push, 2)
            struct HEADER {
                uint32_t padding; // Possible value: 0x00, 0x01, 0x02
                uint16_t unknown[64]; // useless list with values, which increase by 0x4 (maybe global offset list, which is needed for calculating the position of the blocks)
                uint64_t unknown2; //  always 0x00
            };


            struct BLOCK_HEADER {
                uint32_t nVerts;
                uint32_t unknown1;  // Block type? Changes how many padding bytes there are uint16_t[(unknown1 + extraPadByte)*2]
                uint32_t nNormals;  // Extra verts for higher LOD?
                uint32_t nPolygons;
                int32_t position[3]; // Absolute X,Y,Z reference
                int16_t unknown2[4][2]; // No clue
                uint64_t unknown[3]; // Always 0, 1, 1
            };

            // Maybe this is a platform specific VERT HIGH P scenario?
            struct BLOCK_3D {
                int16_t x;
                int16_t y;
                int16_t z;
            };

            struct POLY_3D {
                uint16_t texMap[2]; // [1] seems to be useless. Value of 102 in bottom right of some meshes, small triangle.
                uint16_t vertex[3][4]; // Literally wtf, 3 groups of 4 numbers that look like the vert indexes. One set [1] is usually 0,0,0,0 or 1,1,1,1
                char texName[4];
            };

            struct XBLOCK_1 {
                int16_t unknown[4];
            };

            struct XBLOCK_2 {
                int16_t unknown[4];
            };

            struct XBLOCK_3 {
                int16_t unknown[8];
            };

            struct XBLOCK_4 {
                int16_t unknown[5];
            };

            struct XBLOCK_5 {
                int16_t unknown[9];
            };
#pragma pack(pop)
        };
    };
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

#pragma clang diagnostic pop