#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <cstdint>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Common/NFSVersion.h"

namespace LibOpenNFS {
    namespace NFS2 {
        struct VERT_HIGHP {
            int32_t x, z, y;
        };

        struct ANIM_POS {
            VERT_HIGHP position;
            int16_t unknown[4];
        };

        struct PC {
            struct VERT {
                int16_t x, z, y;
            };

            struct POLYGONDATA {
                int16_t texture;
                int16_t otherSideTex;
                uint8_t vertex[4];
            };

            // TODO: Move this GEO data back to GeoFile.h once I find a clean way to template a shared struct name
#pragma pack(push, 2)
            struct HEADER {
                uint32_t padding;     // Possible value: 0x00, 0x01, 0x02
                uint32_t unknown[32]; // useless list with values, which increase by 0x4 (maybe global offset list, which is needed for
                // calculating the position of the blocks)
                uint64_t unknown2; //  always 0x00
            };

            struct BLOCK_HEADER {
                uint32_t nVerts; // If nVert = 0x00, jump sizeof(GEO_BLOCK_HEADER) forwards, if odd, add 1
                uint32_t nPolygons;
                int32_t position[3]; // Absolute XYZ of the block
                uint16_t unknown;    // ? similar to the value in the list above
                uint16_t unknown1;   // ? similar to the value in the list above
                uint16_t unknown2;   // ? similar to the value in the list above
                uint16_t unknown3;   // ? similar to the value in the list above
                uint64_t pad0;       // always 0x00
                uint64_t pad1;       // always 0x01
                uint64_t pad2;       // always 0x01
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

            // Mike Thompson CarEd disasm parts table for NFS2 Cars
            const std::array<std::string, 32> PART_NAMES = {{
              "High Additional Body Part",
              "High Main Body Part",
              "High Ground Part",
              "High Front Part",
              "High Back Part",
              "High Left Side Part",
              "High Right Side Part",
              "High Additional Left Side Part",
              "High Additional Right Side Part",
              "High Spoiler Part",
              "High Additional Part",
              "High Backlights",
              "High Front Right Wheel",
              "High Front Right Wheel Part",
              "High Front Left Wheel",
              "High Front Left Wheel Part",
              "High Rear Right Wheel",
              "High Rear Right Wheel Part",
              "High Rear Left Wheel",
              "High Rear Left Wheel Part",
              "Medium Additional Body Part",
              "Medium Main Body Part",
              "Medium Ground Part",
              "Low Wheel Part",
              "Low Main Part",
              "Low Side Part",
              "Reserved",
              "Reserved",
              "Reserved",
              "Reserved",
              "Reserved",
              "Reserved",
            }};
        };

        struct PS1 {
            struct VERT {
                int16_t x, z, y, w;
            };

            struct POLYGONDATA {
                uint8_t texture;
                uint8_t otherSideTex;
                uint8_t vertex[4];
            };

            // TODO: Move this GEO data back to GeoFile.h once I find a clean way to template a shared struct name
#pragma pack(push, 2)
            struct HEADER {
                uint32_t padding;     // Possible value: 0x00, 0x01, 0x02
                uint16_t unknown[64]; // useless list with values, which increase by 0x4 (maybe global offset list, which is needed for
                // calculating the position of the blocks)
                uint64_t unknown2; //  always 0x00
            };

            struct BLOCK_HEADER {
                uint32_t nVerts;
                uint32_t unknown1; // Block type? Changes how many padding bytes there are uint16_t[(unknown1 + extraPadByte)*2]
                uint32_t nNormals; // Extra verts for higher LOD?
                uint32_t nPolygons;
                int32_t position[3];    // Absolute X,Y,Z reference
                int16_t unknown2[4][2]; // No clue
                uint64_t unknown[3];    // Always 0, 1, 1
            };

            // Maybe this is a platform specific VERT HIGH P scenario?
            struct BLOCK_3D {
                int16_t x;
                int16_t y;
                int16_t z;
            };

            struct POLY_3D {
                uint16_t texMap[2];    // [1] seems to be useless. Value of 102 in bottom right of some meshes, small triangle.
                uint16_t vertex[3][4]; // Literally wtf, 3 groups of 4 numbers that look like the vert indexes. One set [1] is usually
                // 0,0,0,0 or 1,1,1,1
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

            const std::array<std::string, 33> PART_NAMES = {{"High Additional Body Part",
                                                             "High Main Body Part",
                                                             "High Ground Part",
                                                             "High Front Part",
                                                             "High Rear Part",
                                                             "High Left Side Part",
                                                             "High Right Side Part",
                                                             "High Additional Left Side Part",
                                                             "High Additional Right Side Part",
                                                             "High Front Rear Grilles",
                                                             "High Extra Side Parts",
                                                             "High Spoiler Part",
                                                             "High Additional Part",
                                                             "High Backlights",
                                                             "High Front Right Wheel",
                                                             "High Front Right Wheel Part",
                                                             "High Front Left Wheel",
                                                             "High Front Left Wheel Part",
                                                             "High Rear Right Wheel",
                                                             "High Rear Right Wheel Part",
                                                             "High Rear Left Wheel",
                                                             "High Rear Left Wheel Part",
                                                             "Medium Additional Body Part",
                                                             "Medium Main Body Part",
                                                             "Medium Ground Part",
                                                             "Wheel Positions",
                                                             "Medium/Low Side Parts",
                                                             "Low Main Part",
                                                             "Low Side Part",
                                                             "Headlight Positions",
                                                             "Backlight Positions",
                                                             "Reserved",
                                                             "Reserved"}};
        };
    } // namespace NFS2
} // namespace LibOpenNFS
