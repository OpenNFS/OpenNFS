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
    onfs_check(safe_read(ifstream, blockSize));
    onfs_check(safe_read(ifstream, blockSizeDup));
    onfs_check(safe_read(ifstream, nExtraBlocks));
    onfs_check(safe_read(ifstream, unknown));
    onfs_check(safe_read(ifstream, serialNum));
    onfs_check(safe_read(ifstream, clippingRect, 4 * sizeof(VERT_HIGHP)));
    onfs_check(safe_read(ifstream, extraBlockTblOffset));
    onfs_check(safe_read(ifstream, nStickToNextVerts));
    onfs_check(safe_read(ifstream, nLowResVert));
    onfs_check(safe_read(ifstream, nMedResVert));
    onfs_check(safe_read(ifstream, nHighResVert));
    onfs_check(safe_read(ifstream, nLowResPoly, sizeof(uint16_t)));
    onfs_check(safe_read(ifstream, nMedResPoly, sizeof(uint16_t)));
    onfs_check(safe_read(ifstream, nHighResPoly, sizeof(uint16_t)));
    onfs_check(safe_read(ifstream, unknownPad, 3 * sizeof(uint16_t)));

    // Sanity Checks
    if (blockSize != blockSizeDup) {
        //LOG(DEBUG) << "   --- Bad Block";
        return false;
    }

    // Read 3D Data
    vertexTable.resize(nStickToNextVerts + nHighResVert);
    onfs_check(safe_read(ifstream, vertexTable));

    polygonTable.resize(nLowResPoly + nMedResPoly + nHighResPoly);
    onfs_check(safe_read(ifstream, polygonTable));

    // Read Extrablock data
    ifstream.seekg((uint32_t) trackBlockOffset + 64u + extraBlockTblOffset, std::ios_base::beg);
    // Get extrablock offsets (relative to beginning of TrackBlock)
    extraBlockOffsets.resize(nExtraBlocks);
    onfs_check(safe_read(ifstream, extraBlockOffsets));

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
