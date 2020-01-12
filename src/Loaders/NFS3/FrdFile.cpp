#include "FrdFile.h"

bool FrdFile::LoadFRD(const std::string &frdPath, FrdFile &frdFile)
{
    LOG(INFO) << "Loading FRD File located at " << frdPath;
    std::ifstream frd(frdPath, std::ios::in | std::ios::binary);

    bool loadStatus = frdFile._SerializeIn(frd);
    frd.close();

    return loadStatus;
}

void FrdFile::SaveFRD(const std::string &frdPath, FrdFile &frdFile)
{
    LOG(INFO) << "Saving FRD File to " << frdPath;
    std::ofstream frd(frdPath, std::ios::out | std::ios::binary);
    frdFile.SerializeOut(frd);
}

void FrdFile::MergeFRD(const std::string &frdPath, FrdFile &frdFileA, FrdFile &frdFileB)
{
    // Mergearooney
    // TODO: Of course it couldn't be this simple :(
    frdFileA.nBlocks += frdFileB.nBlocks;
    frdFileA.trackBlocks.insert(frdFileA.trackBlocks.end(), frdFileB.trackBlocks.begin(), frdFileB.trackBlocks.end());
    frdFileA.polygonBlocks.insert(frdFileA.polygonBlocks.end(), frdFileB.polygonBlocks.begin(), frdFileB.polygonBlocks.end());
    frdFileA.extraObjectBlocks.insert(frdFileA.extraObjectBlocks.end(), frdFileB.extraObjectBlocks.begin(), frdFileB.extraObjectBlocks.end());

    FrdFile::SaveFRD(frdPath, frdFileA);
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
    frd.seekg(-4, std::ios_base::cur);

    // Track Data
    for (uint32_t blockIdx = 0; blockIdx < nBlocks; ++blockIdx)
    {
        trackBlocks.push_back(TrkBlock(frd));
    }
    // Geometry
    for (uint32_t blockIdx = 0; blockIdx < nBlocks; ++blockIdx)
    {
        polygonBlocks.push_back(PolyBlock(frd, trackBlocks[blockIdx].nPolygons));
    }
    // Extra Track Geometry
    for (uint32_t blockIdx = 0; blockIdx <= 4 * nBlocks; ++blockIdx)
    {
        extraObjectBlocks.push_back(ExtraObjectBlock(frd));
    }
    // Texture Table
    SAFE_READ(frd, &nTextures, sizeof(uint32_t));
    textureBlocks.reserve(nTextures);
    for (uint32_t tex_Idx = 0; tex_Idx < nTextures; tex_Idx++)
    {
        textureBlocks.push_back(TexBlock(frd));
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

    //frd.write((char *) &ONFS_SIGNATURE, sizeof(uint32_t));

    frd.close();
}


