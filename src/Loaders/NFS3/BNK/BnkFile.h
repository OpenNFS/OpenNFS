#pragma once

#include "../../Common/IRawData.h"
#include "../Common.h"

namespace LibOpenNFS {
    namespace NFS3 {
        class BnkFile : IRawData {
        public:
            BnkFile() = default;
            static bool Load(const std::string &bnkPath, BnkFile &bnkFile);
            static void Save(const std::string &bnkPath, BnkFile &bnkFile);

            char header[4];
            uint16_t version;
            uint16_t numSounds;
            uint32_t firstSoundOffset;
            std::vector<uint32_t> soundOffsets;

        private:
            bool _SerializeIn(std::ifstream &ifstream) override;
            void _SerializeOut(std::ofstream &ofstream) override;
        };
    } // namespace NFS3
} // namespace LibOpenNFS
