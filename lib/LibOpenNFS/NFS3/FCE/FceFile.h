#pragma once

#include "../../Common/IRawData.h"

namespace LibOpenNFS {
    namespace NFS3 {
        struct Colour {
            uint32_t H, S, B, T;
        };

        struct Triangle {
            uint32_t texPage;
            uint32_t vertex[3];  // Local indexes, add part first Vert index from "partFirstVertIndices"
            uint16_t padding[6]; // 00FF
            uint32_t polygonFlags;
            float uvTable[6]; // U1 U2 U3, V1 V2 V3
        };

        struct CarPart {
            std::vector<glm::vec3> vertices;
            std::vector<glm::vec3> normals;
            std::vector<Triangle> triangles;
        };

        class FceFile : IRawData {
        public:
            FceFile() = default;

            static bool Load(const std::string &fcePath, FceFile &fceFile);
            static void Save(const std::string &fcePath, FceFile &fceFile);

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
            glm::vec3 modelHalfSize;
            uint32_t nDummies;
            glm::vec3 dummyCoords[16];
            uint32_t nParts;
            glm::vec3 partCoords[64];
            uint32_t partFirstVertIndices[64];
            uint32_t partNumVertices[64];
            uint32_t partFirstTriIndices[64];
            uint32_t partNumTriangles[64];
            uint32_t nPriColours;
            Colour primaryColours[16];
            uint32_t nSecColours;
            Colour secondaryColours[16];
            char dummyNames[16][64];
            char partNames[64][64];
            uint32_t unknownTable[64];
            std::vector<CarPart> carParts;

        private:
            bool _SerializeIn(std::ifstream &ifstream) override;
            void _SerializeOut(std::ofstream &ofstream) override;
        };
    } // namespace NFS3
} // namespace LibOpenNFS
