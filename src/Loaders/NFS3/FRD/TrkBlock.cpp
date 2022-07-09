#include "TrkBlock.h"

using namespace LibOpenNFS::NFS3;

TrkBlock::TrkBlock(std::ifstream &frd) {
    ASSERT(this->_SerializeIn(frd), "Failed to serialize TrkBlock from file stream");
}

bool TrkBlock::_SerializeIn(std::ifstream &frd) {
    SAFE_READ(frd, &ptCentre, sizeof(glm::vec3));
    SAFE_READ(frd, &ptBounding, sizeof(glm::vec3) * 4);
    SAFE_READ(frd, &nVertices, sizeof(uint32_t));
    SAFE_READ(frd, &nHiResVert, sizeof(uint32_t));
    SAFE_READ(frd, &nLoResVert, sizeof(uint32_t));
    SAFE_READ(frd, &nMedResVert, sizeof(uint32_t));
    SAFE_READ(frd, &nVerticesDup, sizeof(uint32_t));
    SAFE_READ(frd, &nObjectVert, sizeof(uint32_t));

    if (nVertices == 0) {
        return false;
    }

    // Read Vertices
    vert.resize(nVertices);
    SAFE_READ(frd, vert.data(), sizeof(glm::vec3) * nVertices);

    // Read Vertices
    vertShading.resize(nVertices);
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
    posData.resize(nPositions);
    SAFE_READ(frd, posData.data(), sizeof(PositionData) * nPositions);

    // Read virtual road polygons
    polyData.resize(nPolygons);
    SAFE_READ(frd, polyData.data(), sizeof(PolyVRoadData) * nPolygons);

    // Read virtual road spline data
    vroadData.resize(nVRoad);
    SAFE_READ(frd, vroadData.data(), sizeof(VRoadData) * nVRoad);

    // Read Extra object references
    xobj.resize(nXobj);
    SAFE_READ(frd, xobj.data(), sizeof(RefExtraObject) * nXobj);

    // ?? Read unknown
    polyObj.resize(nPolyobj);
    SAFE_READ(frd, polyObj.data(), sizeof(PolyObject) * nPolyobj);
    // nPolyobj = 0;

    // Get the sound and light sources
    soundsrc.resize(nSoundsrc);
    SAFE_READ(frd, soundsrc.data(), sizeof(SoundSource) * nSoundsrc);

    lightsrc.resize(nLightsrc);
    SAFE_READ(frd, lightsrc.data(), sizeof(LightSource) * nLightsrc);

    return true;
}

void TrkBlock::_SerializeOut(std::ofstream &frd) {
    frd.write((char *) &ptCentre, sizeof(glm::vec3));
    frd.write((char *) &ptBounding, sizeof(glm::vec3) * 4);
    frd.write((char *) &nVertices, sizeof(uint32_t));
    frd.write((char *) &nHiResVert, sizeof(uint32_t));
    frd.write((char *) &nLoResVert, sizeof(uint32_t));
    frd.write((char *) &nMedResVert, sizeof(uint32_t));
    frd.write((char *) &nVerticesDup, sizeof(uint32_t));
    frd.write((char *) &nObjectVert, sizeof(uint32_t));
    frd.write((char *) vert.data(), sizeof(glm::vec3) * nVertices);
    frd.write((char *) vertShading.data(), sizeof(uint32_t) * nVertices);
    frd.write((char *) nbdData, 4 * 0x12c);
    frd.write((char *) &nStartPos, sizeof(uint32_t));
    frd.write((char *) &nPositions, sizeof(uint32_t));
    frd.write((char *) &nPolygons, sizeof(uint32_t));
    frd.write((char *) &nVRoad, sizeof(uint32_t));
    frd.write((char *) &nXobj, sizeof(uint32_t));
    frd.write((char *) &nPolyobj, sizeof(uint32_t));
    frd.write((char *) &nSoundsrc, sizeof(uint32_t));
    frd.write((char *) &nLightsrc, sizeof(uint32_t));
    frd.write((char *) posData.data(), sizeof(PositionData) * nPositions);
    frd.write((char *) polyData.data(), sizeof(PolyVRoadData) * nPolygons);
    frd.write((char *) vroadData.data(), sizeof(VRoadData) * nVRoad);
    frd.write((char *) xobj.data(), sizeof(RefExtraObject) * nXobj);
    frd.write((char *) polyObj.data(), sizeof(PolyObject) * nPolyobj);
    frd.write((char *) soundsrc.data(), sizeof(SoundSource) * nSoundsrc);
    frd.write((char *) lightsrc.data(), sizeof(LightSource) * nLightsrc);
}