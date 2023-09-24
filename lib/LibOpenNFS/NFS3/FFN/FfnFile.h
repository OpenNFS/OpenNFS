#pragma once

#include "../../Common/IRawData.h"

namespace LibOpenNFS {
    namespace NFS3 {
        struct HEADER {
            char fntfChk[4];
            uint32_t fileSize;
            uint16_t version;
            uint16_t numChars;
            uint32_t unknown;
            uint16_t zeroPad;
            uint16_t unknown2;
            uint64_t num20Check;
            uint32_t fontMapOffset;
        };

        struct CHAR_TABLE_ENTRY {
            uint16_t asciiCode;
            uint8_t width;
            uint8_t height;
            uint8_t horizontalScroll;
            uint8_t unknown[2];
            uint8_t zeroPad;
            uint8_t rightPadding;
            uint8_t leftPadding;
            uint8_t topPadding;
        };

        class FfnFile : IRawData {
        public:
            FfnFile() = default;

            static bool Load(const std::string &ffnPath, FfnFile &ffnFile);
            static void Save(const std::string &ffnPath, FfnFile &ffnFile);

            HEADER header;
            std::vector<CHAR_TABLE_ENTRY> characters;

        private:
            bool _SerializeIn(std::ifstream &ifstream) override;
            void _SerializeOut(std::ofstream &ofstream) override;
        };
    } // namespace NFS3
} // namespace LibOpenNFS
