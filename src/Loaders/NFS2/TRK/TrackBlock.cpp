#include "TrackBlock.h"

using namespace LibOpenNFS::NFS2;

template <typename Platform>
TrackBlock<Platform>::TrackBlock(std::ifstream &frd)
{
    ASSERT(this->_SerializeIn(frd), "Failed to serialize TrackBlock from file stream");
}

template <typename Platform>
bool TrackBlock<Platform>::_SerializeIn(std::ifstream &ifstream)
{
    // Read Header
    SAFE_READ(ifstream, &blockSize, sizeof(uint32_t));
    SAFE_READ(ifstream, &blockSizeDup, sizeof(uint32_t));
    SAFE_READ(ifstream, &nExtraBlocks, sizeof(uint16_t));
    SAFE_READ(ifstream, &unknown, sizeof(uint16_t));
    SAFE_READ(ifstream, &blockSerial, sizeof(uint32_t));
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
    if (blockSize != blockSizeDup)
    {
        LOG(DEBUG) << "   --- Bad Block";
        return false;
    }

    // Read 3D Data
    vertexTable.reserve(nStickToNextVerts + nHighResVert);
    SAFE_READ(ifstream, vertexTable.data(), (nStickToNextVerts + nHighResVert) * sizeof(typename Platform::VERT));

    polygonTable.reserve(nLowResPoly + nMedResPoly + nHighResPoly);
    SAFE_READ(ifstream, polygonTable.data(), (nLowResPoly + nMedResPoly + nHighResPoly) * sizeof(typename Platform::POLYGONDATA));

    // Read Extrablock data
    // trk.seekg(superblockOffsets[superBlock_Idx] + blockOffsets[block_Idx] + 64u + header->extraBlockTblOffset, std::ios_base::beg);
    // Get extrablock offsets (relative to beginning of TrackBlock)
    extraBlockOffsets.reserve(nExtraBlocks);
    SAFE_READ(ifstream, extraBlockOffsets, nExtraBlocks * sizeof(uint32_t));

    extraObjectBlocks.reserve(nExtraBlocks);

    for (uint32_t xblockIdx = 0; xblockIdx < nExtraBlocks; ++xblockIdx)
    {
        // ifstream.seekg(superblockOffsets[superBlock_Idx] + blockOffsets[block_Idx] + extrablockOffsets[xblockIdx], std::ios_base::beg);
        extraObjectBlocks.push_back(ExtraObjectBlock<Platform>(ifstream));
    }

    return true;
}

template <typename Platform>
void TrackBlock<Platform>::_SerializeOut(std::ofstream &ofstream)
{
    ASSERT(false, "TrackBlock output serialization is not currently implemented");
}