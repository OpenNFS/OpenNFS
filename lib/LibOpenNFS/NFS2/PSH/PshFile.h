#pragma once

#include "../../Common/IRawData.h"

namespace LibOpenNFS::NFS2 {
    class PshFile : IRawData {
        struct HEADER {
            char header[4];        //  "SHPP"
            uint32_t length;       // Inclusive Length of the PSH file
            uint32_t nDirectories; // Number of directory entries
            char chk[4];           // "GIMX"
        };

        struct DIR_ENTRY {
            char imageName[4];
            uint32_t imageOffset; // Offset to start of the image, len implied by difference between offsets to next
        };

        struct IMAGE_HEADER {
            uint8_t imageType; // Image type: Observed values are 0x40, 0x42, 0x43, and 0xC0 The bottom 2 bits of the image type byte specify
            // the bit depth of the image: 0 - 4-bit indexed colour 2 - 16-bit direct colour 3 - 24-bit direct colour
            uint8_t unknown[3];
            uint16_t width;
            uint16_t height;
            uint16_t unknown2[4];
        };

        struct PALETTE_HEADER {
            uint32_t unknown;
            uint16_t paletteWidth;    // Always 16
            uint16_t paletteHeight;   // Always 1
            uint16_t nPaletteEntries; // Always 16
            uint16_t unknown2[3];     // [0] always 0 [1] always 0 [2] often 240, sometimes 0
        };

    public:
        PshFile() = default;

        static bool Load(const std::string &pshPath, PshFile &pshFile);
        static void Save(const std::string &pshPath, PshFile &pshFile);
        static bool Extract(const std::string &outputPath, PshFile &pshFile);

        HEADER header;
        std::vector<DIR_ENTRY> directoryEntries;

    private:
        bool _SerializeIn(std::ifstream &ifstream) override;
        void _SerializeOut(std::ofstream &ofstream) override;
    };
} // namespace LibOpenNFS::NFS2
