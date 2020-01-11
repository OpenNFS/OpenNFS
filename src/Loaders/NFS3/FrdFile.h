#pragma once

#include "RawData.h"

#include "TrkBlock.h"
#include "PolyBlock.h"
#include "ExtraObjectBlock.h"
#include "TexBlock.h"

static const uint8_t HEADER_LENGTH = 28;

class FrdFile : RawData
{
public:
    FrdFile(const std::string &frdPath);
    void SerializeOut(std::ofstream &frd) override;

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
};
