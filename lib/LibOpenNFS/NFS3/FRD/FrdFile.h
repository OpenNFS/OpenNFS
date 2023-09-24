#pragma once

#include "../../Common/IRawData.h"

#include "TrkBlock.h"
#include "PolyBlock.h"
#include "ExtraObjectBlock.h"
#include "TexBlock.h"

namespace LibOpenNFS::NFS3 {
    static const uint8_t HEADER_LENGTH = 28;

    class FrdFile : IRawData {
    public:
        FrdFile() = default;
        static bool Load(const std::string &frdPath, FrdFile &frdFile);
        static void Save(const std::string &frdPath, FrdFile &frdFile);
        static void MergeFRD(const std::string &frdPath, FrdFile &frdFileA, FrdFile &frdFileB);

        // Raw File data
        char header[HEADER_LENGTH];
        uint32_t nBlocks;
        uint32_t nTextures;
        NFSVersion version;
        std::vector<TrkBlock> trackBlocks;
        std::vector<PolyBlock> polygonBlocks;
        std::vector<ExtraObjectBlock> extraObjectBlocks;
        std::vector<TexBlock> textureBlocks;

    private:
        bool _SerializeIn(std::ifstream &ifstream) override;
        void _SerializeOut(std::ofstream &ofstream) override;
    };
} // namespace LibOpenNFS::NFS3
