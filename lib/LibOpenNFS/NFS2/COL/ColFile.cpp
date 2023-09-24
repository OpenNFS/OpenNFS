#include "ColFile.h"

using namespace LibOpenNFS::NFS2;

template <typename Platform>
bool ColFile<Platform>::Load(const std::string &colPath, ColFile &colFile, NFSVersion version) {
    //LOG(INFO) << "Loading COL File located at " << colPath;
    std::ifstream col(colPath, std::ios::in | std::ios::binary);
    colFile.version = version;

    bool loadStatus = colFile._SerializeIn(col);
    col.close();

    return loadStatus;
}

template <typename Platform>
void ColFile<Platform>::Save(const std::string &colPath, ColFile &colFile) {
    //LOG(INFO) << "Saving COL File to " << colPath;
    std::ofstream col(colPath, std::ios::out | std::ios::binary);
    colFile._SerializeOut(col);
}

template <typename Platform>
bool ColFile<Platform>::_SerializeIn(std::ifstream &ifstream) {
    // Check we're in a valid TRK file
    SAFE_READ(ifstream, header, HEADER_LENGTH);
    if (memcmp(header, "COLL", sizeof(header)) != 0)
        return false;

    SAFE_READ(ifstream, &colVersion, sizeof(uint32_t));
    if (colVersion != 11)
        return false;

    SAFE_READ(ifstream, &size, sizeof(uint32_t));
    SAFE_READ(ifstream, &nExtraBlocks, sizeof(uint32_t));

    extraBlockOffsets.resize(nExtraBlocks);
    SAFE_READ(ifstream, extraBlockOffsets.data(), nExtraBlocks * sizeof(uint32_t));

    //LOG(INFO) << "Version: " << colVersion << " nExtraBlocks: " << nExtraBlocks;
    //LOG(DEBUG) << "Parsing COL Extrablocks";

    for (uint32_t extraBlockIdx = 0; extraBlockIdx < nExtraBlocks; ++extraBlockIdx) {
        ifstream.seekg(16 + extraBlockOffsets[extraBlockIdx], std::ios_base::beg);
        extraObjectBlocks.push_back(ExtraObjectBlock<Platform>(ifstream, this->version));
        // Map the the block type to the vector index, gross, original ordering is then maintained for output serialisation
        extraObjectBlockMap[(ExtraBlockID) extraObjectBlocks.back().id] = extraBlockIdx;
    }

    return true;
}

template <typename Platform>
void ColFile<Platform>::_SerializeOut(std::ofstream &ofstream) {
    ASSERT(false, "COL output serialization is not currently implemented");
}
template <typename Platform>
ExtraObjectBlock<Platform> ColFile<Platform>::GetExtraObjectBlock(ExtraBlockID eBlockType) {
    return extraObjectBlocks[extraObjectBlockMap[eBlockType]];
}
template <typename Platform>
bool ColFile<Platform>::IsBlockPresent(ExtraBlockID eBlockType) {
    return extraObjectBlockMap.count(eBlockType);
}

template class LibOpenNFS::NFS2::ColFile<PS1>;
template class LibOpenNFS::NFS2::ColFile<PC>;