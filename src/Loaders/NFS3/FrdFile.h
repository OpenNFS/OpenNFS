#pragma once

#include "IFrdData.h"

#include "TrkBlock.h"
#include "PolyBlock.h"
#include "ExtraObjectBlock.h"
#include "TexBlock.h"

static const uint8_t HEADER_LENGTH = 28;

class FrdFile : IFrdData
{
public:
    FrdFile() = default;
    static bool LoadFRD(const std::string &frdPath, FrdFile &frdFile);
    static void SaveFRD(const std::string &frdPath, FrdFile &frdFile);

    // Raw File data
    char header[HEADER_LENGTH];
    uint32_t nBlocks;
    uint32_t nTextures;
    NFSVer version;
    std::vector<TrkBlock> trackBlocks;
    std::vector<PolyBlock> polygonBlocks;
    std::vector<ExtraObjectBlock> extraObjectBlocks;
    std::vector<TexBlock> textureBlocks;
    uint32_t m_pad;

private:
    bool _SerializeIn(std::ifstream &frd) override;
    void SerializeOut(std::ofstream &frd) override;
};
