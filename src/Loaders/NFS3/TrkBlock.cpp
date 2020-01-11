#include "TrkBlock.h"

TrkBlock::TrkBlock(std::ifstream &frd)
{
    ASSERT(this->_SerializeIn(frd), "Failed to serialize TrkBlock from file stream");
}

bool TrkBlock::_SerializeIn(std::ifstream &frd)
{
    SAFE_READ(frd, &ptCentre, sizeof(glm::vec3));
    SAFE_READ(frd, &ptBounding, sizeof(glm::vec3) * 4);
    SAFE_READ(frd, &nVertices, sizeof(uint32_t));
    SAFE_READ(frd, &nHiResVert, sizeof(uint32_t));
    SAFE_READ(frd, &nLoResVert, sizeof(uint32_t));
    SAFE_READ(frd, &nMedResVert, sizeof(uint32_t));
    SAFE_READ(frd, &nVerticesDup, sizeof(uint32_t));
    SAFE_READ(frd, &nObjectVert, sizeof(uint32_t));

    if (nVertices == 0) return false;

    // Read Vertices
    vert.reserve(nVertices);
    SAFE_READ(frd, vert.data(), sizeof(glm::vec3) * nVertices);

    // Read Vertices
    vertShading.reserve(nVertices);
    SAFE_READ(frd, vertShading.data(), sizeof(uint32_t) * nVertices);

    // Read neighbouring block data
    SAFE_READ(frd, nbdData, 4 * 0x12c);

    // Read trackblock metadata
    SAFE_READ(frd, &nStartPos, sizeof(uint32_t));
    SAFE_READ(frd, &nPositions, sizeof(uint32_t));
    SAFE_READ(frd, &nPolygons, sizeof(uint32_t));
    SAFE_READ(frd, &nVRoad, sizeof(uint32_t));
    SAFE_READ(frd, &nXobj, sizeof(uint32_t));
    SAFE_READ(frd, &nPolyobj, sizeof(uint32_t));
    SAFE_READ(frd, &nSoundsrc, sizeof(uint32_t));
    SAFE_READ(frd, &nLightsrc, sizeof(uint32_t));

    // Read track position data
    posData.reserve(nPositions);
    SAFE_READ(frd, posData.data(), sizeof(PositionData) * nPositions);

    // Read virtual road polygons
    polyData.reserve(nPolygons);
    SAFE_READ(frd, polyData.data(), sizeof(PolyVRoadData) * nPolygons);

    // Read virtual road spline data
    vroadData.reserve(nVRoad);
    SAFE_READ(frd, vroadData.data(), sizeof(VRoadData) * nVRoad);

    if (nXobj > 0)
    {
        xobj.reserve(nXobj);
        SAFE_READ(frd, xobj.data(), sizeof(RefExtraObject) * nXobj);
    }

    if (nPolyobj > 0)
    {
        frd.seekg(20 * nPolyobj, std::ios_base::cur);
    }
    nPolyobj = 0;

    if (nSoundsrc > 0)
    {
        soundsrc.reserve(nSoundsrc);
        SAFE_READ(frd, soundsrc.data(), sizeof(SoundSource) * nSoundsrc);
    }

    if (nLightsrc > 0)
    {
        lightsrc.reserve(nLightsrc);
        SAFE_READ(frd, lightsrc.data(), sizeof(LightSource) * nLightsrc);
    }

    return true;
}

void TrkBlock::SerializeOut(std::ofstream &frd)
{
    ASSERT(false, "TrkBlock serialization to file stream is not implemented");
}