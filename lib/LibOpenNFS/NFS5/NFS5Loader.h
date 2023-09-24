#pragma once

#include <fstream>
#include <string>
#include <cstring>
#include "glm/gtc/type_ptr.hpp"
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include "../../../src/Physics/Car.h"
#include "../../nfs_data.h"
#include "../../../src/Util/Utils.h"
#include "../../../src/Util/ImageLoader.h"
#include "../../../include/CrpLib/Lib.h"

using namespace CrpLib;
using namespace Utils;

// Derived from Arushans collated Addict and Jesper-Juul Mortensen notes
// Deprecated in favour of CrpLib
struct CRP {
    struct HEADER_INFO {
        uint32_t data; // First 5 bits unknown (0x1A), last 27 bits == Num Parts
        uint32_t getNumParts() {
            return data >> 5;
        }
    };

    struct LENGTH_INFO {
        uint32_t data;
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
        uint32_t offset;          // Relative from current article offset * 16. Points to a PART_TABLE
    };

    enum PartType { MiscPart, MaterialPart, FshPart, BasePart, NamePart, CullingPart, TransformationPart, VertexPart, NormalPart, UVPart, TrianglePart, EffectPart };

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
        uint32_t offset;  // Relative from current MATERIAL_PART offset
    };

    struct FSH_PART {
        uint16_t index;
        char identifier[2]; // ('fs'/'sf')
        LENGTH_INFO lengthInfo;
        uint32_t nFshFiles; // Seems to always be 0x01?
        uint32_t offset;    // Relative from current FSH_PART offset
    };

    struct PART_INFO_A {
        uint16_t data;
    };

    struct PART_INFO_B {
        uint16_t data;
    };

    struct BASE_PART {
        char identifier[4];     // ('esaB'/'Base')
        LENGTH_INFO lengthInfo; // Length
        uint32_t unknown;       // (seems always to be 0x00000000)
        uint32_t offset;        // Relative from current BASE_PART offset
    };

    struct NAME_PART {
        char identifier[4];     // ('emaN'/'Name')
        LENGTH_INFO lengthInfo; // Text length
        uint32_t unknown;       // (seems always to be 0x00000000)
        uint32_t offset;        // Relative from current NAME_PART offset
    };

    struct CULLING_PART {
        PART_INFO_A partInfo;
        char identifier[2]; // ('n$'/'$n')
        LENGTH_INFO lengthInfo;
        uint32_t nCullingParts; // (seems always to be 0x00000000)
        uint32_t offset;        // Relative from current CULLING_PART offset
    };

    struct TRANSFORMATION_PART {
        PART_INFO_A partInfo;
        char identifier[2]; // ('rt'/'tr')
        LENGTH_INFO lengthInfo;
        uint32_t nTransformMatrices; // Always 1
        uint32_t offset;             // Relative from current TRANSFORMATION_PART offset
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
        PartType getPartType() {
            if ((strncmp(materialPart.identifier, "tm", 2) == 0) || strncmp(materialPart.identifier, "mt", 2) == 0) {
                return PartType::MaterialPart;
            } else if ((strncmp(fshPart.identifier, "fs", 2) == 0) || strncmp(fshPart.identifier, "sf", 2) == 0) {
                return PartType::FshPart;
            } else if ((strncmp(basePart.identifier, "esaB", 4) == 0) || strncmp(basePart.identifier, "Base", 4) == 0) {
                return PartType::BasePart;
            } else if ((strncmp(namePart.identifier, "emaN", 4) == 0) || strncmp(namePart.identifier, "Name", 4) == 0) {
                return PartType::NamePart;
            } else if ((strncmp(cullingPart.identifier, "n$", 2) == 0) || strncmp(cullingPart.identifier, "$n", 2) == 0) {
                return PartType::CullingPart;
            } else if ((strncmp(transformationPart.identifier, "rt", 2) == 0) || strncmp(transformationPart.identifier, "tr", 2) == 0) {
                return PartType::TransformationPart;
            } else if ((strncmp(vertexPart.identifier, "tv", 2) == 0) || strncmp(vertexPart.identifier, "vt", 2) == 0) {
                return PartType::VertexPart;
            } else if ((strncmp(normalPart.identifier, "mn", 2) == 0) || strncmp(normalPart.identifier, "nm", 2) == 0) {
                return PartType::NormalPart;
            } else if ((strncmp(uvPart.identifier, "vu", 2) == 0) || strncmp(uvPart.identifier, "uv", 2) == 0) {
                return PartType::UVPart;
            } else if ((strncmp(trianglePart.identifier, "rp", 2) == 0) || strncmp(trianglePart.identifier, "pr", 2) == 0) {
                return PartType::TrianglePart;
            } else if ((strncmp(effectPart.identifier, "fe", 2) == 0) || strncmp(effectPart.identifier, "ef", 2) == 0) {
                return PartType::EffectPart;
            } else {
                return PartType::MiscPart;
            }
        }
    };

    // ONFS Helper structure
    // TODO: It's probably time to get OOP with this...
    struct ARTICLE_DATA {
        explicit ARTICLE_DATA(uint32_t articleIdx) : index(articleIdx){};
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
};

class NFS5 {
public:
    static std::shared_ptr<Car> LoadCar(const std::string &carBasePath);
    static CarData LoadCRP(const std::string &crpPath);

private:
    // OpenNFS derived method of dumping FSH textures. To be Deprecated eventually in favour of CrpLib mechanisms
    static void DumpCrpTextures(const std::string &crpPath);
    static void DumpArticleVertsToObj(CRP::ARTICLE_DATA article);
};
