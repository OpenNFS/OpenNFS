#include "FrdFile.h"

FrdFile::FrdFile(const std::string &frdPath)
{
    LOG(INFO) << "Loading FRD File located at " << frdPath;
    std::ifstream frd(frdPath, std::ios::in | std::ios::binary);

    ASSERT(this->_SerializeIn(frd), "Failed to serialize Frd from file stream");

    frd.close();
}

bool FrdFile::_SerializeIn(std::ifstream &frd)
{
    SAFE_READ(frd, header, HEADER_LENGTH);
    SAFE_READ(frd, &nBlocks, sizeof(uint32_t));
    ++nBlocks;

    if (nBlocks < 1 || nBlocks > 500)
    {
        return false;
    }

    trackBlocks.reserve(nBlocks);
    polygonBlocks.reserve(nBlocks);
    extraObjectBlocks.reserve((4 * nBlocks) + 1);

    // Detect NFS3 or NFSHS
    int32_t hsMagic = 0;
    SAFE_READ(frd, &hsMagic, sizeof(int32_t));

    if ((hsMagic < 0) || (hsMagic > 5000))
    {
        version = NFSVer::NFS_3;
    }
    else if (((hsMagic + 7) / 8) == nBlocks)
    {
        version = NFSVer::NFS_4;
    }
    else
    {
        // Unknown file type
        return false;
    }

    // Back up a little, as this sizeof(int32_t) into a trackblock that we're about to deserialize
    frd.seekg(-sizeof(int32_t), std::ios_base::cur);

    // Track Data
    for (uint32_t blockIdx = 0; blockIdx < nBlocks; ++blockIdx)
    {
        trackBlocks.emplace_back(TrkBlock(frd));
    }
    // Geometry
    for (uint32_t blockIdx = 0; blockIdx < nBlocks; ++blockIdx)
    {
        polygonBlocks.emplace_back(PolyBlock(frd, trackBlocks[blockIdx].nPolygons));
    }
    // Extra Track Geometry
    for (uint32_t blockIdx = 0; blockIdx < nBlocks; ++blockIdx)
    {
        extraObjectBlocks.emplace_back(ExtraObjectBlock(frd));
    }
    // Texture Table
    SAFE_READ(frd, &nTextures, sizeof(uint32_t));
    textureBlocks.reserve(nTextures);
    for (uint32_t tex_Idx = 0; tex_Idx < nTextures; tex_Idx++)
    {
        textureBlocks.emplace_back(TexBlock(frd));
    }

    return true;
}

void FrdFile::SerializeOut(std::ofstream &frd)
{
    // Write FRD Header
    frd.write((char *) &header, HEADER_LENGTH);
    uint32_t nBlocksHeader = nBlocks - 1;
    frd.write((char*) &nBlocksHeader, sizeof(uint32_t));

    // Track Data
    for(auto &trackBlock : trackBlocks)
    {
        trackBlock.SerializeOut(frd);
    }
    // Geometry
    for (auto &polyBlock : polygonBlocks)
    {
        polyBlock.SerializeOut(frd);
    }
    // Extra Track Geometry
    for (auto &extraObjectBlock : extraObjectBlocks)
    {
        extraObjectBlock.SerializeOut(frd);
    }
    // Texture Table
    frd.write((char*) &nTextures, sizeof(uint32_t));
    for (auto &textureBlock : textureBlocks)
    {
        textureBlock.SerializeOut(frd);
    }

    frd.write((char *) &m_pad, sizeof(uint32_t));

    frd.close();
}
