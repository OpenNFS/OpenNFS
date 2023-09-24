#include "FfnFile.h"

//#include "../../../../src/Util/ImageLoader.h"

using namespace LibOpenNFS::NFS3;

bool FfnFile::Load(const std::string &ffnPath, FfnFile &ffnFile) {
    //LOG(INFO) << "Loading FFN File located at " << ffnPath;
    std::ifstream ffn(ffnPath, std::ios::in | std::ios::binary);

    bool loadStatus = ffnFile._SerializeIn(ffn);
    ffn.close();

    return loadStatus;
}

void FfnFile::Save(const std::string &ffnPath, FfnFile &ffnFile) {
    //LOG(INFO) << "Saving FFN File to " << ffnPath;
    std::ofstream ffn(ffnPath, std::ios::out | std::ios::binary);
    ffnFile._SerializeOut(ffn);
}

bool FfnFile::_SerializeIn(std::ifstream &ifstream) {
    // Get filesize so can check have parsed all bytes
    SAFE_READ(ifstream, &header, sizeof(HEADER));

    if (memcmp(header.fntfChk, "FNTF", sizeof(header.fntfChk)) != 0) {
        //LOG(WARNING) << "Invalid FFN Header";
        return false;
    }

    characters.resize(header.numChars);
    SAFE_READ(ifstream, characters.data(), sizeof(CHAR_TABLE_ENTRY) * header.numChars);

    uint32_t predictedAFontOffset = header.fontMapOffset;

    header.numChars = 400;
    header.version  = 164;

    // streamoff readBytes = ffn.tellg();
    // ASSERT(readBytes == header->fileSize, "Missing " << header.fileSize - readBytes << " bytes from loaded FFN file: " << ffn_path);

    ifstream.seekg(header.fontMapOffset, std::ios_base::beg);
    std::vector<uint32_t> pixels(header.version * header.numChars);
    std::vector<uint16_t> paletteColours(0xFF);
    std::vector<uint8_t> indices(header.version * header.numChars); // Only used if indexed

    for (int pal_Idx = 0; pal_Idx < 255; ++pal_Idx) {
        paletteColours[pal_Idx] = 65535;
    }

    for (int y = 0; y < header.numChars; y++) {
        for (int x = 0; x < header.version; x++) {
            SAFE_READ(ifstream, &indices[(x + y * header.version)], sizeof(uint8_t));
        }
    }

    // Rewrite the pixels using the palette data
    for (int y = 0; y < header.numChars; y++) {
        for (int x = 0; x < header.version; x++) {
            uint32_t pixel                   = 0;//ImageLoader::abgr1555ToARGB8888(paletteColours[indices[(x + y * header.version)]]);
            pixels[(x + y * header.version)] = pixel;
        }
    }

    //ImageLoader::SaveImage("C:/Users/Amrik/Desktop/test.bmp", pixels.data(), header.version, header.numChars);

    // ASSERT(readBytes == header.fileSize, "Missing " << header.fileSize - readBytes << " bytes from loaded FFN file: " << ffn_path);
    return true;
}

void FfnFile::_SerializeOut(std::ofstream &ofstream) {
    ASSERT(false, "FFN output serialization is not currently implemented");
}
