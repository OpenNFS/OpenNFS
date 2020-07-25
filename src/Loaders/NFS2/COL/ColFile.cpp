#include "ColFile.h"

using namespace LibOpenNFS::NFS2;

template class LibOpenNFS::NFS2::ColFile<PS1>;
template class LibOpenNFS::NFS2::ColFile<PC>;

template <typename Platform>
bool ColFile<Platform>::Load(const std::string &colPath, ColFile &colFile)
{
    LOG(INFO) << "Loading COL File located at " << colPath;
    std::ifstream col(colPath, std::ios::in | std::ios::binary);

    bool loadStatus = colFile._SerializeIn(col);
    col.close();

    return loadStatus;
}

template <typename Platform>
void ColFile<Platform>::Save(const std::string &colPath, ColFile &colFile)
{
    LOG(INFO) << "Saving COL File to " << colPath;
    std::ofstream col(colPath, std::ios::out | std::ios::binary);
    colFile._SerializeOut(col);
}

template <typename Platform>
bool ColFile<Platform>::_SerializeIn(std::ifstream &ifstream)
{
    // Check we're in a valid TRK file
    SAFE_READ(ifstream, header, HEADER_LENGTH);
    if (memcmp(header, "COLL", sizeof(header)) != 0)
        return false;

    SAFE_READ(ifstream, &version, sizeof(uint32_t));
    if (version != 11)
        return false;

    SAFE_READ(ifstream, &size, sizeof(uint32_t));
    SAFE_READ(ifstream, &nExtraBlocks, sizeof(uint32_t));

    extraBlockOffsets.reserve(nExtraBlocks);
    SAFE_READ(ifstream, extraBlockOffsets.data(), nExtraBlocks * sizeof(uint32_t));

    LOG(INFO) << "Version: " << version << " nExtraBlocks: " << nExtraBlocks;
    LOG(DEBUG) << "Parsing COL Extrablocks";

    for (uint32_t xBlock_Idx = 0; xBlock_Idx < nExtraBlocks; ++xBlock_Idx)
    {
        ifstream.seekg(16 + extraBlockOffsets[xBlock_Idx], std::ios_base::beg);
        ExtraObjectBlock<Platform> extraObjectBlock = ExtraObjectBlock<Platform>(ifstream);
        extraObjectBlocks[(ExtraBlockID)extraObjectBlock.id] = extraObjectBlock;
    }

    return true;
}

template <typename Platform>
void ColFile<Platform>::_SerializeOut(std::ofstream &ofstream)
{
    ASSERT(false, "COL output serialization is not currently implemented");
}
