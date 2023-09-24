#pragma once

#include "../../Common/IRawData.h"

namespace LibOpenNFS {
    namespace NFS3 {
        struct PositionData // enumerate polygons which lie at center
        {
            uint16_t polygon;
            unsigned char nPolygons;
            char unknown;
            int16_t extraNeighbor1, extraNeighbor2;
        };

        struct NeighbourData // info on neighbouring block numbers
        {
            int16_t blk, unknown;
        };

        struct PolyVRoadData // vroad data associated with a polygon
        {
            unsigned char vroadEntry;
            unsigned char flags;
            unsigned char unknown[6];
        };

        struct VRoadData // vroad vectors
        {
            uint16_t xNorm, zNorm, yNorm;
            uint16_t xForw, zForw, yForw;
        };

        struct RefExtraObject // description of a block's XOBJects.
        {
            glm::ivec3 pt;
            uint16_t unknown1;
            uint16_t globalno; // sequence number in all of the track's xobjs
            uint16_t unknown2;
            char crossindex; // position in first POLYOBJ chunk (0 if not in first chunk)
            char unknown3;
        }; // !!! does not list the animated XOBJs

        struct SoundSource {
            glm::ivec3 refpoint;
            uint32_t type;
        };

        struct LightSource {
            glm::ivec3 refpoint;
            uint32_t type;
        };

        struct PolyObject {
            uint8_t unknown[20];
        };

        class TrkBlock : public IRawData {
        public:
            TrkBlock() = default;
            explicit TrkBlock(std::ifstream &frd);
            void _SerializeOut(std::ofstream &frd) override;

            glm::vec3 ptCentre;
            glm::vec3 ptBounding[4];
            uint32_t nVertices;                           // Total num verties in block
            uint32_t nHiResVert, nLoResVert, nMedResVert; // LOD Vert numbers
            uint32_t nVerticesDup, nObjectVert;
            std::vector<glm::vec3> vert;
            std::vector<uint32_t> vertShading;
            NeighbourData nbdData[0x12C]; // neighboring blocks
            uint32_t nStartPos, nPositions;
            uint32_t nPolygons, nVRoad, nXobj, nPolyobj, nSoundsrc, nLightsrc;
            std::vector<PositionData> posData;   // positions auint32_t track
            std::vector<PolyVRoadData> polyData; // polygon vroad references & flags
            std::vector<VRoadData> vroadData;    // vroad vectors
            std::vector<RefExtraObject> xobj;
            std::vector<PolyObject> polyObj; // Unknown Currently!
            std::vector<SoundSource> soundsrc;
            std::vector<LightSource> lightsrc;
            glm::vec3 hs_ptMin, hs_ptMax;
            uint32_t hs_neighbors[8];

        protected:
            bool _SerializeIn(std::ifstream &frd) override;
        };
    } // namespace NFS3
} // namespace LibOpenNFS
