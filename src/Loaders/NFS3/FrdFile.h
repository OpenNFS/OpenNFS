#pragma once

#include "IFrdData.h"

#include "TrkBlock.h"
#include "PolyBlock.h"
#include "ExtraObjectBlock.h"
#include "TexBlock.h"

static const uint8_t HEADER_LENGTH = 28;
static const uint32_t ONFS_SIGNATURE = 0x15B001C0;

class FrdFile : IFrdData
{
public:
    FrdFile() = default;
    static bool LoadFRD(const std::string &frdPath, FrdFile &frdFile);
    static void SaveFRD(const std::string &frdPath, FrdFile &frdFile);
    static void MergeFRD(const std::string &frdPath, FrdFile &frdFileA, FrdFile &frdFileB);

    // Raw File data
    char header[HEADER_LENGTH];
    uint32_t nBlocks;
    uint32_t nTextures;
    NFSVer version;
    std::vector<TrkBlock> trackBlocks;
    std::vector<PolyBlock> polygonBlocks;
    std::vector<ExtraObjectBlock> extraObjectBlocks;
    std::vector<TexBlock> textureBlocks;

private:
    bool _SerializeIn(std::ifstream &frd) override;
    void SerializeOut(std::ofstream &frd) override;
};
