#include "SuperBlock.h"

using namespace LibOpenNFS::NFS2;

template <typename Platform>
SuperBlock<Platform>::SuperBlock(std::ifstream &trk)
{
    ASSERT(this->_SerializeIn(trk), "Failed to serialize SuperBlock from file stream");
}

template <typename Platform>
bool SuperBlock<Platform>::_SerializeIn(std::ifstream &ifstream)
{
    std::streampos superblockOffset = ifstream.tellg();
    SAFE_READ(ifstream, superBlockSize, sizeof(uint32_t));
    SAFE_READ(ifstream, nBlocks, sizeof(uint32_t));
    SAFE_READ(ifstream, padding, sizeof(uint32_t));

    if (nBlocks != 0)
    {
        // Get the offsets of the child blocks within superblock
        blockOffsets.reserve(nBlocks);
        SAFE_READ(ifstream, blockOffsets.data(), nBlocks * sizeof(uint32_t));
        trackBlocks.reserve(nBlocks);

        for (uint32_t blockIdx = 0; blockIdx < nBlocks; ++blockIdx)
        {
            // LOG(DEBUG) << "  Block " << block_Idx + 1 << " of " << superblock->nBlocks << " [" << trackblock->header->blockSerial << "]";
            // TODO: Is this seek necessary?
            // ifstream.seekg(superblockOffset + blockOffsets[blockIdx], std::ios_base::beg);
            trackBlocks.push_back(TrackBlock<Platform>(ifstream));
        }
    }

    return true;
}

template <typename Platform>
void SuperBlock<Platform>::_SerializeOut(std::ofstream &ofstream)
{
    ASSERT(false, "SuperBlock output serialization is not currently implemented");
}