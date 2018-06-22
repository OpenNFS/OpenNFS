//
// Created by Amrik.Sadhra on 20/06/2018.
//

#include <iomanip>
#include "trk_loader.h"


std::map<short, GLuint> GenTrackTextures(std::map<short, Texture> textures) {
    std::map<short, GLuint> gl_id_map;

    for (auto it = textures.begin(); it != textures.end(); ++it) {
        Texture texture = it->second;
        GLuint textureID;
        glGenTextures(1, &textureID);
        gl_id_map[it->first] = textureID;
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // TODO: Use Filtering for Textures with no alpha component
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, texture.width, texture.height, GL_RGBA, GL_UNSIGNED_BYTE,
                          (const GLvoid *) texture.texture_data);
    }

    return gl_id_map;
}

std::vector<short>
RemapTextureIDs(const std::set<short> &minimal_texture_ids_set, std::vector<unsigned int> &texture_indices) {
    // Get ordered list of unique texture id's present in block
    std::vector<short> texture_ids;
    texture_ids.assign(minimal_texture_ids_set.begin(), minimal_texture_ids_set.end());
    // Remap Normals to correspond to ordered texture ID's
    std::map<int, int> ordered_mapping;
    for (int t = 0; t < texture_ids.size(); ++t) {
        ordered_mapping[texture_ids[t]] = t;
    }
    for (auto &texture_index : texture_indices) {
        texture_index = static_cast<unsigned int>(ordered_mapping.find(texture_index)->second);
    }
    return texture_ids;
}

bool ExtractTrackTextures(const std::string &track_path, const ::std::string track_name, NFSVer nfs_version) {
    std::stringstream output_dir, tex_archive_path;
    std::string psh_path = track_path;
    output_dir << TRACK_PATH;

    switch (nfs_version) {
        case NFS_2:
            output_dir << "NFS2/";
            tex_archive_path << track_path << "0.qfs";
            break;
        case NFS_2_SE:
            output_dir << "NFS2_SE/";
            tex_archive_path << track_path << "0M.qfs";
            break;
        case NFS_3:
            output_dir << "NFS3/";
            tex_archive_path << track_path << "0.qfs";
            break;
        case NFS_3_PS1:
            psh_path.replace(psh_path.find("ZZ"), 2, "");
            output_dir << "NFS3_PS1/";
            tex_archive_path << psh_path << "0.PSH";
            break;
        case UNKNOWN:
        default:
            output_dir << "UNKNOWN/";
            break;
    }
    output_dir << track_name;

    if (boost::filesystem::exists(output_dir.str())) {
        return true;
    } else {
        boost::filesystem::create_directories(output_dir.str());
    }

    std::cout << "Extracting track textures" << std::endl;

    if (nfs_version == NFS_3_PS1) {
        output_dir << "/textures/";
        return ExtractPSH(tex_archive_path.str(), output_dir.str());
    } else if (nfs_version == NFS_3) {
        std::stringstream sky_fsh_path;
        sky_fsh_path << track_path.substr(0, track_path.find_last_of('/')) << "/sky.fsh";
        if (boost::filesystem::exists(sky_fsh_path.str())) {
            std::stringstream sky_textures_path;
            sky_textures_path << output_dir.str() << "/sky_textures/";
            std::cout << sky_fsh_path.str() << std::endl;
            ASSERT(ExtractQFS(sky_fsh_path.str(), sky_textures_path.str()),
                   "Unable to extract sky textures from sky.fsh");
        }
    }

    output_dir << "/textures/";
    return (ExtractQFS(tex_archive_path.str(), output_dir.str()));
}

uint32_t abgr1555ToARGB8888(uint16_t abgr1555) {
    uint8_t red = static_cast<int>(round((abgr1555 & 0x1F) / 31.0F * 255.0F));
    uint8_t green = static_cast<int>(round(((abgr1555 & 0x3E0) >> 5) / 31.0F * 255.0F));
    uint8_t blue = static_cast<int>(round(((abgr1555 & 0x7C00) >> 10) / 31.0F * 255.0F));

    uint32_t alpha = 255;
    if (((abgr1555 & 0x8000) == 0 ? 1 : 0) == ((red == 0) && (green == 0) && (blue == 0) ? 1 : 0)) {
        alpha = 0;
    }

    return alpha << 24 | red << 16 | green << 8 | blue;
}

// lpBits stand for long pointer bits
// szPathName : Specifies the pathname        -> the file path to save the image
// lpBits    : Specifies the bitmap bits      -> the buffer (content of the) image
// w    : Specifies the image width
// h    : Specifies the image height
bool SaveImage(const char *szPathName, void *lpBits, uint16_t w, uint16_t h) {
    // Create a new file for writing
    FILE *pFile = fopen(szPathName, "wb"); // wb -> w: writable b: binary, open as writable and binary
    if (pFile == NULL) {
        return false;
    }

    BITMAPINFOHEADER BMIH;                         // BMP header
    BMIH.biSize = sizeof(BITMAPINFOHEADER);
    BMIH.biSizeImage = w * h * 3;
    // Create the bitmap for this OpenGL context
    BMIH.biSize = sizeof(BITMAPINFOHEADER);
    BMIH.biWidth = w;
    BMIH.biHeight = h;
    BMIH.biPlanes = 1;
    BMIH.biBitCount = 32;
    BMIH.biCompression = BI_RGB;
    BMIH.biSizeImage = w * h * 4;

    BITMAPFILEHEADER bmfh;                         // Other BMP header
    int nBitsOffset = sizeof(BITMAPFILEHEADER) + BMIH.biSize;
    LONG lImageSize = BMIH.biSizeImage;
    LONG lFileSize = nBitsOffset + lImageSize;
    bmfh.bfType = 'B' + ('M' << 8);
    bmfh.bfOffBits = nBitsOffset;
    bmfh.bfSize = lFileSize;
    bmfh.bfReserved1 = bmfh.bfReserved2 = 0;

    // Write the bitmap file header               // Saving the first header to file
    uint32_t nWrittenFileHeaderSize = fwrite(&bmfh, 1, sizeof(BITMAPFILEHEADER), pFile);

    // And then the bitmap info header            // Saving the second header to file
    uint32_t nWrittenInfoHeaderSize = fwrite(&BMIH, 1, sizeof(BITMAPINFOHEADER), pFile);

    // Finally, write the image data itself
    //-- the data represents our drawing          // Saving the file content in lpBits to file
    uint32_t nWrittenDIBDataSize = fwrite(lpBits, 1, lImageSize, pFile);
    fclose(pFile); // closing the file.


    return true;
}

bool ExtractPSH(const std::string &psh_path, const std::string &output_path) {
    using namespace NFS2;
    boost::filesystem::create_directories(output_path);
    std::cout << "Extracting PSH File " << std::endl;
    ifstream psh(psh_path, ios::in | ios::binary);

    PS1::PSH::HEADER *pshHeader = new PS1::PSH::HEADER();

    // Check we're in a valid TRK file
    if (psh.read(((char *) pshHeader), sizeof(PS1::PSH::HEADER)).gcount() != sizeof(PS1::PSH::HEADER)) {
        std::cout << "Couldn't open file/truncated." << std::endl;
        delete pshHeader;
        return false;
    }

    std::cout << pshHeader->nDirectories << " images inside PSH" << std::endl;

    // Header should contain TRAC
    if (memcmp(pshHeader->header, "SHPP", sizeof(pshHeader->header)) != 0 &&
        memcmp(pshHeader->chk, "GIMX", sizeof(pshHeader->chk)) != 0) {
        std::cout << "Invalid PSH Header(s)." << std::endl;
        delete pshHeader;
        return false;
    }

    // Get the offsets to each image in the PSH
    auto *directoryEntries = new PS1::PSH::DIR_ENTRY[pshHeader->nDirectories];
    psh.read(((char *) directoryEntries), pshHeader->nDirectories * sizeof(PS1::PSH::DIR_ENTRY));

    for (int image_Idx = 0; image_Idx < pshHeader->nDirectories; ++image_Idx) {
        std::cout << "Extracting GIMX " << image_Idx << std::endl;
        psh.seekg(directoryEntries[image_Idx].imageOffset, ios_base::beg);
        auto *imageHeader = new PS1::PSH::IMAGE_HEADER();
        psh.read(((char *) imageHeader), sizeof(PS1::PSH::IMAGE_HEADER));

        uint8_t bitDepth = static_cast<uint8_t>(imageHeader->imageType & 0x3);
        uint32_t pixels[imageHeader->width * imageHeader->height];
        uint8_t *indexPair = new uint8_t();
        uint8_t *indexes = new uint8_t[imageHeader->width * imageHeader->height]; // Only used if indexed
        bool hasAlpha = false;
        bool isPadded = false;
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
                        uint32_t pixel = abgr1555ToARGB8888(*input);
                        hasAlpha = (pixel & 0xFF000000) != -16777216;
                        pixels[(x + y * imageHeader->width)] = pixel;
                        break;
                    }
                    case 3: { // 24-bit direct colour
                        uint8_t alpha = 255u;
                        uint8_t rgb[3];
                        psh.read((char *) rgb, 3 * sizeof(uint8_t));
                        if ((rgb[0] == 0) && (rgb[1] == 0) && (rgb[2] == 0)) {
                            hasAlpha = true;
                            alpha = 0;
                        }
                        pixels[(x + y * imageHeader->width)] = (alpha << 24 | rgb[0] << 16 | rgb[1] << 8 | rgb[2]);
                    }
                }
                if ((x == imageHeader->width - 1) && (isPadded)) {
                    psh.seekg(1, ios_base::cur); // Skip a byte of padding
                }
            }
        }

        // We only have to look up a Palette if an indexed type
        if (bitDepth == 0 or bitDepth == 1) {
            auto *paletteHeader = new PS1::PSH::PALETTE_HEADER();
            psh.read((char *) paletteHeader, sizeof(PS1::PSH::PALETTE_HEADER));
            if (paletteHeader->paletteHeight != 1) {
                // There is padding, search for a '1' in the paletteHeader as this is constant as the height of all paletteHeaders,
                // then jump backwards by how offset 'height' is into paletteHeader to get proper
                psh.seekg(-(signed)sizeof(PS1::PSH::PALETTE_HEADER), ios_base::cur);
                if (paletteHeader->unknown == 1) { //8 bytes early
                    psh.seekg(-8, ios_base::cur);
                } else if (paletteHeader->paletteWidth == 1) { // 4 bytes early
                    psh.seekg(-4, ios_base::cur);
                } else if (paletteHeader->nPaletteEntries == 1) { // 2 bytes late
                    psh.seekg(2, ios_base::cur);
                } else if (paletteHeader->unknown2[0] == 1) { // 4 bytes late
                    psh.seekg(4, ios_base::cur);
                } else if (paletteHeader->unknown2[1] == 1) { // 6 bytes late
                    psh.seekg(6, ios_base::cur);
                } else if (paletteHeader->unknown2[2] == 1) { //8 bytes late
                    psh.seekg(8, ios_base::cur);
                } else {
                    // TODO: Well damn. It's padded a lot further out. Do a uint16 '1' search, then for a '16' or '256' imm following
                }
                psh.read((char *) paletteHeader, sizeof(PS1::PSH::PALETTE_HEADER));
            }

            // Read Palette
            if (paletteHeader->nPaletteEntries == 0) {
                return false;
            }

            uint16_t *paletteColours = new uint16_t[paletteHeader->nPaletteEntries];
            psh.read((char *) paletteColours, paletteHeader->nPaletteEntries * sizeof(uint16_t));

            // Rewrite the pixels using the palette data
            if ((bitDepth == 0) || (bitDepth == 1)) {
                for (int y = 0; y < imageHeader->height; y++) {
                    for (int x = 0; x < imageHeader->width; x++) {
                        uint32_t pixel = abgr1555ToARGB8888(paletteColours[indexes[(x + y * imageHeader->width)]]);
                        pixels[(x + y * imageHeader->width)] = pixel;
                    }
                }
            }
        }
        std::stringstream output_bmp;
        output_bmp << output_path << setfill('0') << setw(4) << image_Idx << ".BMP";;
        SaveImage(output_bmp.str().c_str(), &pixels, imageHeader->width, imageHeader->height);
    }

    delete pshHeader;
    return true;
}

bool ExtractQFS(const std::string &qfs_input, const std::string &output_dir) {
    // Fshtool molests the current working directory, save and restore
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));

    char *args[3] = {const_cast<char *>(""), strdup(qfs_input.c_str()), strdup(output_dir.c_str())};
    int returnCode = (fsh_main(3, args) == 1);

    chdir(cwd);

    return returnCode;
}