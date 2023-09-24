#include "TrackBlock.h"

using namespace LibOpenNFS::NFS2;

template <typename Platform>
TrackBlock<Platform>::TrackBlock(std::ifstream &trk, NFSVersion version) {
    this->version = version;
    ASSERT(this->_SerializeIn(trk), "Failed to serialize TrackBlock from file stream");
}

template <typename Platform>
bool TrackBlock<Platform>::_SerializeIn(std::ifstream &ifstream) {
    std::streampos trackBlockOffset = ifstream.tellg();
    // Read Header
    SAFE_READ(ifstream, &blockSize, sizeof(uint32_t));
    SAFE_READ(ifstream, &blockSizeDup, sizeof(uint32_t));
    SAFE_READ(ifstream, &nExtraBlocks, sizeof(uint16_t));
    SAFE_READ(ifstream, &unknown, sizeof(uint16_t));
    SAFE_READ(ifstream, &serialNum, sizeof(uint32_t));
    SAFE_READ(ifstream, clippingRect, 4 * sizeof(VERT_HIGHP));
    SAFE_READ(ifstream, &extraBlockTblOffset, sizeof(uint32_t));
    SAFE_READ(ifstream, &nStickToNextVerts, sizeof(uint16_t));
    SAFE_READ(ifstream, &nLowResVert, sizeof(uint16_t));
    SAFE_READ(ifstream, &nMedResVert, sizeof(uint16_t));
    SAFE_READ(ifstream, &nHighResVert, sizeof(uint16_t));
    SAFE_READ(ifstream, &nLowResPoly, sizeof(uint16_t));
    SAFE_READ(ifstream, &nMedResPoly, sizeof(uint16_t));
    SAFE_READ(ifstream, &nHighResPoly, sizeof(uint16_t));
    SAFE_READ(ifstream, unknownPad, 3 * sizeof(uint16_t));

    // Sanity Checks
    if (blockSize != blockSizeDup) {
        //LOG(DEBUG) << "   --- Bad Block";
        return false;
    }

    // Read 3D Data
    vertexTable.resize(nStickToNextVerts + nHighResVert);
    SAFE_READ(ifstream, vertexTable.data(), (nStickToNextVerts + nHighResVert) * sizeof(typename Platform::VERT));

    polygonTable.resize(nLowResPoly + nMedResPoly + nHighResPoly);
    SAFE_READ(ifstream, polygonTable.data(), (nLowResPoly + nMedResPoly + nHighResPoly) * sizeof(typename Platform::POLYGONDATA));

    // Read Extrablock data
    ifstream.seekg((uint32_t) trackBlockOffset + 64u + extraBlockTblOffset, std::ios_base::beg);
    // Get extrablock offsets (relative to beginning of TrackBlock)
    extraBlockOffsets.resize(nExtraBlocks);
    SAFE_READ(ifstream, extraBlockOffsets.data(), nExtraBlocks * sizeof(uint32_t));

    for (uint32_t extraBlockIdx = 0; extraBlockIdx < nExtraBlocks; ++extraBlockIdx) {
        ifstream.seekg((uint32_t) trackBlockOffset + extraBlockOffsets[extraBlockIdx], std::ios_base::beg);
        extraObjectBlocks.push_back(ExtraObjectBlock<Platform>(ifstream, this->version));
        // Map the the block type to the vector index, original ordering is then maintained for output serialisation
        extraObjectBlockMap[(ExtraBlockID) extraObjectBlocks.back().id] = extraBlockIdx;
    }

    return true;
}

template <typename Platform>
void TrackBlock<Platform>::_SerializeOut(std::ofstream &ofstream) {
    ASSERT(false, "TrackBlock output serialization is not currently implemented");
}

template <typename Platform>
ExtraObjectBlock<Platform> TrackBlock<Platform>::GetExtraObjectBlock(ExtraBlockID eBlockType) {
    return extraObjectBlocks[extraObjectBlockMap[eBlockType]];
}

template <typename Platform>
bool TrackBlock<Platform>::IsBlockPresent(ExtraBlockID eBlockType) {
    return extraObjectBlockMap.count(eBlockType);
}

template class LibOpenNFS::NFS2::TrackBlock<PS1>;
template class LibOpenNFS::NFS2::TrackBlock<PC>;
