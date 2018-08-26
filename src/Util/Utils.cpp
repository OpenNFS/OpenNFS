//
// Created by Amrik Sadhra on 11/03/2018.
//

#include "Utils.h"
#include "../nfs_data.h"


namespace Utils {
    glm::vec3 bulletToGlm(const btVector3 &v) { return glm::vec3(v.getX(), v.getY(), v.getZ()); }

    btVector3 glmToBullet(const glm::vec3 &v) { return btVector3(v.x, v.y, v.z); }

    glm::quat bulletToGlm(const btQuaternion &q) {
        return glm::quat(q.getW(), q.getX(), q.getY(), q.getZ());
    }

    btQuaternion glmToBullet(const glm::quat &q) { return btQuaternion(q.x, q.y, q.z, q.w); }

    btMatrix3x3 glmToBullet(const glm::mat3 &m) {
        return btMatrix3x3(m[0][0], m[1][0], m[2][0], m[0][1], m[1][1], m[2][1], m[0][2], m[1][2], m[2][2]);
    }

    btBoxShape *genCollisionBox(std::vector<glm::vec3> model_vertices) {
        glm::vec3 bottom_left = glm::vec3(model_vertices[0].x, model_vertices[0].y, model_vertices[0].z);
        glm::vec3 top_right = glm::vec3(model_vertices[0].x, model_vertices[0].y, model_vertices[0].z);


        for (auto &vertex : model_vertices) {
            if (vertex.x < bottom_left.x) {
                bottom_left.x = vertex.x;
            }
            if (vertex.y < bottom_left.y) {
                bottom_left.y = vertex.y;
            }
            if (vertex.z < bottom_left.z) {
                bottom_left.z = vertex.z;
            }
            if (vertex.x > top_right.x) {
                top_right.x = vertex.x;
            }
            if (vertex.y > top_right.y) {
                top_right.y = vertex.y;
            }
            if (vertex.z > top_right.z) {
                top_right.z = vertex.z;
            }
        }

        return new btBoxShape(
                btVector3((top_right.x - bottom_left.x) / 2, (top_right.y - bottom_left.y) / 2,
                          (top_right.z - bottom_left.z) / 2));
    }

    glm::vec3 genDimensions(std::vector<glm::vec3> model_vertices) {
        glm::vec3 bottom_left = glm::vec3(model_vertices[0].x, model_vertices[0].y, model_vertices[0].z);
        glm::vec3 top_right = glm::vec3(model_vertices[0].x, model_vertices[0].y, model_vertices[0].z);


        for (auto &vertex : model_vertices) {
            if (vertex.x < bottom_left.x) {
                bottom_left.x = vertex.x;
            }
            if (vertex.y < bottom_left.y) {
                bottom_left.y = vertex.y;
            }
            if (vertex.z < bottom_left.z) {
                bottom_left.z = vertex.z;
            }
            if (vertex.x > top_right.x) {
                top_right.x = vertex.x;
            }
            if (vertex.y > top_right.y) {
                top_right.y = vertex.y;
            }
            if (vertex.z > top_right.z) {
                top_right.z = vertex.z;
            }
        }

        return glm::vec3((top_right.x - bottom_left.x) / 2, (top_right.y - bottom_left.y) / 2,
                         (top_right.z - bottom_left.z) / 2);
    }

    unsigned int endian_swap(unsigned int x) {
        int swapped;

        swapped = (x >> 24) |
                  ((x << 8) & 0x00FF0000) |
                  ((x >> 8) & 0x0000FF00) |
                  (x << 24);

        return static_cast<unsigned int>(swapped);
    }

    unsigned int readInt32(FILE *file, bool littleEndian) {
        int data = fgetc(file);
        data = data << 8 | fgetc(file);
        data = data << 8 | fgetc(file);
        data = data << 8 | fgetc(file);

        return littleEndian ? endian_swap(static_cast<unsigned int>(data)) : data;
    }

    // Move these to a native resource handler class
    std::vector<CarModel> LoadOBJ(std::string obj_path) {
        std::vector<CarModel> meshes;

        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string err;
        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, obj_path.c_str())) {
            std::cout << err << std::endl;
            return meshes;
        }
        // Loop over shapes
        for (size_t s = 0; s < shapes.size(); s++) {
            std::vector<glm::vec3> verts = std::vector<glm::vec3>();
            std::vector<glm::vec3> norms = std::vector<glm::vec3>();
            std::vector<glm::vec2> uvs = std::vector<glm::vec2>();
            std::vector<unsigned int> indices = std::vector<unsigned int>();
            // Loop over faces(polygon)
            size_t index_offset = 0;
            for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
                int fv = shapes[s].mesh.num_face_vertices[f];
                // Loop over vertices in the face.
                for (size_t v = 0; v < fv; v++) {
                    // access to vertex
                    tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                    indices.emplace_back((const unsigned int &) idx.vertex_index);

                    verts.emplace_back(glm::vec3(attrib.vertices[3 * idx.vertex_index + 0] * 0.1,
                                                 attrib.vertices[3 * idx.vertex_index + 1] * 0.1,
                                                 attrib.vertices[3 * idx.vertex_index + 2] * 0.1));
                    norms.emplace_back(
                            glm::vec3(attrib.normals[3 * idx.normal_index + 0], attrib.normals[3 * idx.normal_index + 1],
                                      attrib.normals[3 * idx.normal_index + 2]));
                    uvs.emplace_back(glm::vec2(attrib.texcoords[2 * idx.texcoord_index + 0],
                                               1.0f - attrib.texcoords[2 * idx.texcoord_index + 1]));
                }
                index_offset += fv;
                // per-face material
                shapes[s].mesh.material_ids[f];
            }
            CarModel obj_mesh = CarModel(shapes[s].name + "_obj", verts, uvs, norms, indices, glm::vec3(0, 0, 0), 0.01, 0.0f, 0.5);
            meshes.emplace_back(obj_mesh);
        }
        return meshes;
    }

    bool ExtractVIV(const std::string &viv_path, const std::string &output_dir) {
        FILE *vivfile, *outfile;  // file stream
        int numberOfFiles; // number of files in viv file
        char filename[100];
        int filepos, filesize;
        char buf[4];
        int c;
        int pos;
        long currentpos;
        int a, b;

        if(boost::filesystem::exists(output_dir)){
            std::cout << "VIV already extracted." << std::endl;
            return true;
        } else {
            boost::filesystem::create_directories(output_dir);
        }

        vivfile = fopen(viv_path.c_str(), "rb");

        if (!vivfile) {
            printf("Error while opening %s\n", viv_path.c_str());
            return false;
        }

        for (a = 0; a < 4; a++) {
            buf[a] = fgetc(vivfile);
        }

        if (memcmp(buf, "BIGF", 4)) {
            printf("%s if not a viv file\n", viv_path.c_str());
            fclose(vivfile);
            return false;
        }

        readInt32(vivfile, false); // size of viv file

        numberOfFiles = readInt32(vivfile, false);
        printf("Number of files: %d\n", numberOfFiles);

        int startPos = readInt32(vivfile, false); // start position of files

        currentpos = ftell(vivfile);

        for (a = 0; a < numberOfFiles; a++) {
            if (fseek(vivfile, currentpos, SEEK_SET)) {
                printf("Error while seeking in file %s\n", viv_path.c_str());
                fclose(vivfile);
                return false;
            }

            filepos = readInt32(vivfile, false);
            filesize = readInt32(vivfile, false);

            pos = 0;
            c = fgetc(vivfile);
            while (c != '\0') {
                filename[pos] = (char) c;
                pos++;
                c = fgetc(vivfile);
            }
            filename[pos] = '\0';

            currentpos = ftell(vivfile);

            // TODO: Can place into a subdirectory using car name, wherever it's stored.
            std::stringstream out_file_path;
            out_file_path << output_dir << filename;

            if ((outfile = fopen(out_file_path.str().c_str(), "wb")) == NULL) {
                printf("Error while opening file %s\n", filename);
                fclose(vivfile);
                return false;
            }

            if (fseek(vivfile, filepos, SEEK_SET)) {
                printf("Error while seeking in file %s\n", viv_path.c_str());
                fclose(vivfile);
                return false;
            }

            for (b = 0; b < filesize; b++) {
                c = fgetc(vivfile);
                fputc(c, outfile);
            }

            fclose(outfile);
            printf("File %s written successfully\n", filename);
        }
        fclose(vivfile);

        return true;
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

    bool ExtractQFS(const std::string &qfs_input, const std::string &output_dir) {
        if (boost::filesystem::exists(output_dir)) {
            std::cout << "Textures already exist at " << output_dir << " so returning." << std::endl;
            return true;
        }

        boost::filesystem::create_directories(output_dir);

        // Fshtool molests the current working directory, save and restore
        char cwd[1024];
        getcwd(cwd, sizeof(cwd));

        char *args[3] = {const_cast<char *>(""), strdup(qfs_input.c_str()), strdup(output_dir.c_str())};
        int returnCode = (fsh_main(3, args) == 1);

        chdir(cwd);

        return returnCode;
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

    bool ExtractPSH(const std::string &psh_path, const std::string &output_path) {
        using namespace NFS2_DATA;

        if (boost::filesystem::exists(output_path)) {
            std::cout << "Textures already exist at " << output_path << " so returning." << std::endl;
            return true;
        }

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
            std::cout << "Extracting GIMX " << image_Idx << ": " << directoryEntries[image_Idx].imageName[0] << directoryEntries[image_Idx].imageName[1] << directoryEntries[image_Idx].imageName[2] << directoryEntries[image_Idx].imageName[3] << ".BMP" << std::endl;
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
                    psh.seekg(-(signed) sizeof(PS1::PSH::PALETTE_HEADER), ios_base::cur);
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
                        ASSERT(false, "Couldn't find palette header for file " << psh_path);
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
            //output_bmp << output_path << setfill('0') << setw(4) << image_Idx << ".BMP";
            output_bmp << output_path << directoryEntries[image_Idx].imageName[0] << directoryEntries[image_Idx].imageName[1] << directoryEntries[image_Idx].imageName[2] << directoryEntries[image_Idx].imageName[3] << ".BMP";
            SaveImage(output_bmp.str().c_str(), &pixels, imageHeader->width, imageHeader->height);
        }

        delete pshHeader;
        return true;
    }

    // TODO: Integrate into LoadBmpCustomAlpha as a master bitmap loader
    bool LoadBmpCustomAlpha(const char *fname, GLubyte **bits, GLsizei *width_, GLsizei *height_, uint8_t alphaColour) {
        GLsizei width, height;
        bool retval = false;
        // load file and check if it looks reasonable
        FILE *fp = fopen(fname, "rb");
        if (fp) {
            fseek(fp, 0L, 2);
            long size = ftell(fp);
            if (size > (long) sizeof(BITMAPFILEHEADER)) {
                unsigned char *data = new unsigned char[size];
                if (data) {
                    fseek(fp, 0L, 0);
                    if (fread(data, size, 1, fp) == 1) {
                        BITMAPFILEHEADER *file_header = (BITMAPFILEHEADER *) data;
                        if (file_header->bfType == MAKEWORD('B', 'M')) {
                            if (file_header->bfSize == (DWORD) size) {
                                BITMAPINFO *info = (BITMAPINFO *) (data + sizeof(BITMAPFILEHEADER));
                                // we only handle uncompressed bitmaps
                                if (info->bmiHeader.biCompression == BI_RGB) {
                                    width = info->bmiHeader.biWidth;
                                    *width_ = width;
                                    if (width > 0) {
                                        height = info->bmiHeader.biHeight;
                                        *height_ = height;
                                        if (height) {
                                            if (height < 0) height = (-height);
                                            // we want RGBA. let's alloc enough space
                                            *bits = new GLubyte[width * height * 4L];
                                            if (*bits) {
                                                retval = true;
                                                GLubyte *current_bits = *bits;
                                                GLubyte *pixel = data + file_header->bfOffBits;
                                                GLsizei h = height, w = width;
                                                long padding;
                                                switch (info->bmiHeader.biBitCount) {
                                                    // 8-bit palette bitmaps
                                                    case 8:
                                                        padding = w % 2;
                                                        RGBQUAD rgba;
                                                        for (; h > 0; h--) {
                                                            for (w = width; w > 0; w--) {
                                                                rgba = info->bmiColors[*pixel];
                                                                pixel++;
                                                                *current_bits++ = rgba.rgbRed;
                                                                *current_bits++ = rgba.rgbGreen;
                                                                *current_bits++ = rgba.rgbBlue;
                                                                if(rgba.rgbRed == 0 && rgba.rgbGreen == alphaColour && rgba.rgbBlue == 0){
                                                                    *current_bits++ = 0;
                                                                } else {
                                                                    *current_bits++ = 255;
                                                                }
                                                            }
                                                            pixel += padding;
                                                        }
                                                        break;
                                                        // 24-bit bitmaps
                                                    case 24:
                                                        padding = (w * 3) % 2;
                                                        for (; h > 0; h--) {
                                                            for (w = width; w > 0; w--) {
                                                                *current_bits++ = pixel[2];
                                                                *current_bits++ = pixel[1];
                                                                *current_bits++ = pixel[0];
                                                                if(pixel[2] == 0 && pixel[1] == alphaColour && pixel[0] == 0){
                                                                    *current_bits++ = 0;
                                                                } else {
                                                                    *current_bits++ = 255;
                                                                }
                                                                pixel += 3;
                                                            }
                                                            pixel += padding;
                                                        }
                                                        break;
                                                    case 32:
                                                        // 32-bit bitmaps
                                                        // never seen it, but Win32 SDK claims the existance
                                                        // of that value. 4th byte is assumed to be alpha-channel.
                                                        for (; h > 0; h--) {
                                                            for (w = width; w > 0; w--) {
                                                                *current_bits++ = pixel[2];
                                                                *current_bits++ = pixel[1];
                                                                *current_bits++ = pixel[0];
                                                                if(pixel[2] == 0 && pixel[1] == alphaColour && pixel[0] == 0){
                                                                    *current_bits++ = 0;
                                                                } else {
                                                                    *current_bits++ = pixel[3];
                                                                }
                                                                pixel += 4;
                                                            }
                                                        }
                                                        break;// I don't like 1,4 and 16 bit.
                                                    default:
                                                        delete[] *bits;
                                                        retval = false;
                                                        break;
                                                }
                                                if (retval) {
                                                    if (info->bmiHeader.biHeight < 0) {
                                                        long *data_q = (long *) *bits;
                                                        long wt = width * 4L;
                                                        long *dest_q = (long *) (*bits + (height - 1) * wt);
                                                        long tmp;
                                                        while (data_q < dest_q) {
                                                            for (w = width; w > 0; w--) {
                                                                tmp = *data_q;
                                                                *data_q++ = *dest_q;
                                                                *dest_q++ = tmp;
                                                            }
                                                            dest_q -= (wt + wt);
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    delete[] data;
                }
            }
            fclose(fp);
        }
        return retval;
    }

    bool LoadBmpWithAlpha(const char *fname, const char *afname, GLubyte **bits, GLsizei width, GLsizei height) {
        bool retval = false;
        // load file and check if it looks reasonable
        FILE *fp = fopen(fname, "rb");
        FILE *fp_a = fopen(afname, "rb");
        if (fp && fp_a) {
            fseek(fp, 0L, 2);
            fseek(fp_a, 0L, 2);
            long size = ftell(fp);
            long size_a = ftell(fp_a);
            unsigned char *data = new unsigned char[size];
            unsigned char *data_a = new unsigned char[size_a];
            if (data && data_a) {
                fseek(fp, 0L, 0);
                fseek(fp_a, 0L, 0);
                if ((fread(data, size, 1, fp) == 1) && (fread(data_a, size_a, 1, fp_a) == 1)) {
                    BITMAPFILEHEADER *file_header = (BITMAPFILEHEADER *) data;
                    BITMAPFILEHEADER *file_header_a = (BITMAPFILEHEADER *) data_a;
                    if (file_header->bfType == MAKEWORD('B', 'M')) {
                        if (file_header->bfSize == (DWORD) size) {
                            BITMAPINFO *info = (BITMAPINFO *) (data +
                                                               sizeof(BITMAPFILEHEADER));// we only handle uncompressed bitmaps
                            BITMAPINFO *info_a = (BITMAPINFO *) (data_a +
                                                                 sizeof(BITMAPFILEHEADER));// we only handle uncompressed bitmaps
                            if (info->bmiHeader.biCompression == BI_RGB) {
                                width = info->bmiHeader.biWidth;
                                if (width > 0) {
                                    height = info->bmiHeader.biHeight;
                                    if (height) {
                                        if (height < 0)
                                            height = (-height);// we want RGBA. let's alloc enough space
                                        *
                                                bits = new GLubyte[width * height * 4L];
                                        if (*bits) {
                                            retval = true;
                                            GLubyte *current_bits = *bits;
                                            GLubyte *pixel = data + file_header->bfOffBits;
                                            GLubyte *pixel_a = data_a + file_header_a->bfOffBits;
                                            GLsizei h = height, w = width;
                                            long padding, padding_a;
                                            switch (info->bmiHeader.biBitCount) {// 24-bit bitmaps
                                                case 8:
                                                    padding_a = w % 2;
                                                    padding = w % 2;
                                                    RGBQUAD rgba;
                                                    for (; h > 0; h--) {
                                                        for (w = width; w > 0; w--) {
                                                            rgba = info->bmiColors[*pixel];
                                                            pixel++;
                                                            pixel_a++;
                                                            *current_bits++ = rgba.rgbRed;
                                                            *current_bits++ = rgba.rgbGreen;
                                                            *current_bits++ = rgba.rgbBlue;
                                                            *current_bits++ = rgba.rgbRed;
                                                        }
                                                        pixel += padding;
                                                        pixel_a += padding_a;
                                                    }
                                                    break;
                                                case 24:
                                                    // Read the 8 Bit bitmap alpha data
                                                    padding_a = w % 2;
                                                    padding = (w * 3) % 2;
                                                    for (; h > 0; h--) {
                                                        for (w = width; w > 0; w--) {
                                                            rgba = info_a->bmiColors[*pixel_a];
                                                            pixel_a++;
                                                            *current_bits++ = pixel[2];
                                                            *current_bits++ = pixel[1];
                                                            *current_bits++ = pixel[0];
                                                            *current_bits++ = rgba.rgbRed;
                                                            pixel += 3;
                                                        }
                                                        pixel += padding;
                                                        pixel_a += padding_a;
                                                    }
                                                    break;
                                                case 32:
                                                    // 32-bit bitmaps
                                                    // never seen it, but Win32 SDK claims the existance
                                                    // of that value. 4th byte is assumed to be alpha-channel.
                                                    for (; h > 0; h--) {
                                                        for (w = width; w > 0; w--) {
                                                            *current_bits++ = pixel[2];
                                                            *current_bits++ = pixel[1];
                                                            *current_bits++ = pixel[0];
                                                            *current_bits++ = pixel[3];
                                                            pixel += 4;
                                                        }
                                                    }
                                                    break;// I don't like 1,4 and 16 bit.
                                                default:
                                                    delete[] *bits;
                                                    retval = false;
                                                    break;
                                            }
                                            if (retval) {// mirror image if neccessary (never tested)
                                                if (info->bmiHeader.biHeight < 0) {
                                                    long *data_q = (long *) *bits;
                                                    long wt = width * 4L;
                                                    long *dest_q = (long *) (*bits + (height - 1) * wt);
                                                    long tmp;
                                                    while (data_q < dest_q) {
                                                        for (w = width; w > 0; w--) {
                                                            tmp = *data_q;
                                                            *data_q++ = *dest_q;
                                                            *dest_q++ = tmp;
                                                        }
                                                        dest_q -= (wt + wt);
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                delete[] data;
                delete[] data_a;
            }
            fclose(fp);
            fclose(fp_a);
        }
        return retval;
    }
}


