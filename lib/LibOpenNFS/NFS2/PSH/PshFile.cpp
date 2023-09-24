#include "PshFile.h"

#include "Common/TextureUtils.h"

// TODO: Need to perform proper deserialisation in this file, and then use a helper in ImageLoader that calls into this class
// #include "../../../Util/ImageLoader.h"

using namespace LibOpenNFS::NFS2;

bool PshFile::Load(const std::string &pshPath, PshFile &pshFile) {
    // LOG(INFO) << "Loading PSH File located at " << pshPath;
    std::ifstream psh(pshPath, std::ios::in | std::ios::binary);

    bool loadStatus = pshFile._SerializeIn(psh);
    psh.close();

    return loadStatus;
}

void PshFile::Save(const std::string &pshPath, PshFile &pshFile) {
    // LOG(INFO) << "Saving PSH File to " << pshPath;
    std::ofstream psh(pshPath, std::ios::out | std::ios::binary);
    pshFile._SerializeOut(psh);
}

bool PshFile::_SerializeIn(std::ifstream &ifstream) {
    // Check we're in a valid PSH file
    SAFE_READ(ifstream, &header, sizeof(HEADER));

    // LOG(INFO) << header.nDirectories << " images inside PSH";

    // Header should contain SHPP
    if (memcmp(header.header, "SHPP", sizeof(header.header)) != 0 && memcmp(header.chk, "GIMX", sizeof(header.chk)) != 0) {
        // LOG(WARNING) << "Invalid PSH Header(s)";
        return false;
    }

    // Get the offsets to each image in the PSH
    directoryEntries.resize(header.nDirectories);
    SAFE_READ(ifstream, directoryEntries.data(), header.nDirectories * sizeof(DIR_ENTRY));

    return true;
}

bool PshFile::Extract(const std::string &outputPath, PshFile &pshFile) {
    // LOG(INFO) << "Extracting PSH file: " << psh_path << " to " << output_path;

    /*if (std::filesystem::exists(outputPath)) {
        // LOG(INFO) << "Textures already exist at " << output_path << ". Nothing to extract";
        return true;
    }

    std::filesystem::create_directories(outputPath);
    std::ifstream psh(psh_path, std::ios::in | std::ios::binary);

    HEADER *pshHeader = new HEADER();

    // Check we're in a valid TRK file
    if (psh.read(((char *) pshHeader), sizeof(HEADER)).gcount() != sizeof(HEADER)) {
        // LOG(WARNING) << "Couldn't open file/truncated";
        delete pshHeader;
        return false;
    }

    // LOG(INFO) << pshHeader->nDirectories << " images inside PSH";

    // Header should contain TRAC
    if (memcmp(pshHeader->header, "SHPP", sizeof(pshHeader->header)) != 0 && memcmp(pshHeader->chk, "GIMX", sizeof(pshHeader->chk)) != 0) {
        // LOG(WARNING) << "Invalid PSH Header(s)";
        delete pshHeader;
        return false;
    }

    // Get the offsets to each image in the PSH
    auto *directoryEntries = new DIR_ENTRY[pshHeader->nDirectories];
    psh.read(((char *) directoryEntries), pshHeader->nDirectories * sizeof(DIR_ENTRY));

    for (uint32_t image_Idx = 0; image_Idx < pshHeader->nDirectories; ++image_Idx) {
        // LOG(INFO) << "Extracting GIMX " << image_Idx << ": " << directoryEntries[image_Idx].imageName[0] << directoryEntries[image_Idx].imageName[1]
        //           << directoryEntries[image_Idx].imageName[2] << directoryEntries[image_Idx].imageName[3] << ".BMP";
        psh.seekg(directoryEntries[image_Idx].imageOffset, std::ios_base::beg);
        auto *imageHeader = new IMAGE_HEADER();
        psh.read(((char *) imageHeader), sizeof(IMAGE_HEADER));

        uint8_t bitDepth   = static_cast<uint8_t>(imageHeader->imageType & 0x3);
        uint32_t *pixels   = new uint32_t[imageHeader->width * imageHeader->height];
        uint8_t *indexPair = new uint8_t();
        uint8_t *indexes   = new uint8_t[imageHeader->width * imageHeader->height]; // Only used if indexed
        bool hasAlpha      = false;
        bool isPadded      = false;
        if (bitDepth == 0) {
            isPadded = (imageHeader->width % 4 == 1) || (imageHeader->width % 4 == 2);
        } else if (bitDepth == 1 || bitDepth == 3) {
            isPadded = imageHeader->width % 2 == 1;
        }

        for (int y = 0; y < imageHeader->height; y++) {
            for (int x = 0; x < imageHeader->width; x++) {
                switch (bitDepth) {
                case 0: { // 4-bit indexed colour
                    uint8_t index;
                    if (x % 2 == 0) {
                        psh.read((char *) indexPair, sizeof(uint8_t));
                        index = static_cast<uint8_t>(*indexPair & 0xF);
                    } else {
                        index = *indexPair >> 4;
                    }
                    indexes[(x + y * imageHeader->width)] = index;
                    break;
                }
                case 1: { // 8-bit indexed colour
                    psh.read((char *) &indexes[(x + y * imageHeader->width)], sizeof(uint8_t));
                    break;
                }
                case 2: { // 16-bit direct colour
                    uint16_t *input = new uint16_t;
                    psh.read((char *) input, sizeof(uint16_t));
                    uint32_t pixel                       = TextureUtils::abgr1555ToARGB8888(*input);
                    hasAlpha                             = (pixel & 0xFF000000) != -16777216;
                    pixels[(x + y * imageHeader->width)] = pixel;
                    break;
                }
                case 3: { // 24-bit direct colour
                    uint8_t alpha = 255u;
                    uint8_t rgb[3];
                    psh.read((char *) rgb, 3 * sizeof(uint8_t));
                    if ((rgb[0] == 0) && (rgb[1] == 0) && (rgb[2] == 0)) {
                        hasAlpha = true;
                        alpha    = 0;
                    }
                    pixels[(x + y * imageHeader->width)] = (alpha << 24 | rgb[0] << 16 | rgb[1] << 8 | rgb[2]);
                }
                }
                if ((x == imageHeader->width - 1) && (isPadded)) {
                    psh.seekg(1, std::ios_base::cur); // Skip a byte of padding
                }
            }
        }

        // We only have to look up a Palette if an indexed type
        if (bitDepth == 0 || bitDepth == 1) {
            auto *paletteHeader = new PALETTE_HEADER();
            psh.read((char *) paletteHeader, sizeof(PALETTE_HEADER));
            if (paletteHeader->paletteHeight != 1) {
                // There is padding, search for a '1' in the paletteHeader as this is constant as the height of all paletteHeaders,
                // then jump backwards by how offset 'height' is into paletteHeader to get proper
                psh.seekg(-(signed) sizeof(PALETTE_HEADER), std::ios_base::cur);
                if (paletteHeader->unknown == 1) { // 8 bytes early
                    psh.seekg(-8, std::ios_base::cur);
                } else if (paletteHeader->paletteWidth == 1) { // 4 bytes early
                    psh.seekg(-4, std::ios_base::cur);
                } else if (paletteHeader->nPaletteEntries == 1) { // 2 bytes late
                    psh.seekg(2, std::ios_base::cur);
                } else if (paletteHeader->unknown2[0] == 1) { // 4 bytes late
                    psh.seekg(4, std::ios_base::cur);
                } else if (paletteHeader->unknown2[1] == 1) { // 6 bytes late
                    psh.seekg(6, std::ios_base::cur);
                } else if (paletteHeader->unknown2[2] == 1) { // 8 bytes late
                    psh.seekg(8, std::ios_base::cur);
                } else {
                    ASSERT(false, "Couldn't find palette header for file " << psh_path);
                    // TODO: Well damn. It's padded a lot further out. Do a uint16 '1' search, then for a '16' or '256' imm following
                }
                psh.read((char *) paletteHeader, sizeof(PALETTE_HEADER));
            }

            // Read Palette
            if (paletteHeader->nPaletteEntries == 0) {
                delete paletteHeader;
                return false;
            }

            uint16_t *paletteColours = new uint16_t[paletteHeader->nPaletteEntries];
            psh.read((char *) paletteColours, paletteHeader->nPaletteEntries * sizeof(uint16_t));

            // Rewrite the pixels using the palette data
            if ((bitDepth == 0) || (bitDepth == 1)) {
                for (int y = 0; y < imageHeader->height; y++) {
                    for (int x = 0; x < imageHeader->width; x++) {
                        uint32_t pixel                       = LibOpenNFS::TextureUtils::abgr1555ToARGB8888(paletteColours[indexes[(x + y * imageHeader->width)]]);
                        pixels[(x + y * imageHeader->width)] = pixel;
                    }
                }
            }

            delete paletteHeader;
        }
        std::stringstream output_bmp;
        // output_bmp << output_path << setfill('0') << setw(4) << image_Idx << ".BMP";
        output_bmp << output_path << directoryEntries[image_Idx].imageName[0] << directoryEntries[image_Idx].imageName[1] << directoryEntries[image_Idx].imageName[2]
                   << directoryEntries[image_Idx].imageName[3] << ".BMP";
        SaveImage(output_bmp.str().c_str(), pixels, imageHeader->width, imageHeader->height);
        delete[] pixels;
    }

    delete pshHeader;*/
    return true;
}

void PshFile::_SerializeOut(std::ofstream &ofstream) {
    ASSERT(false, "GEO output serialization is not currently implemented");
}
