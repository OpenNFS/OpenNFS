#include "Utils.h"

#include <glm/gtx/quaternion.hpp>
#include <filesystem>
#include <sstream>

namespace LibOpenNFS::Utils {
    glm::vec3 FixedToFloat(glm::vec3 fixedPoint) {
        return fixedPoint / 65536.0f;
    }

    // Modified Arushan CRP decompressor. Removes LZ77 style decompression from CRPs
    bool DecompressCRP(const std::string &compressedCrpPath, const std::string &decompressedCrpPath) {
        // LOG(INFO) << "Decompressing CRP File located at " << compressedCrpPath;

        // Bail early if decompressed CRP present already
        if (std::filesystem::exists(decompressedCrpPath)) {
            // LOG(INFO) << "Already decompressed, skipping";
            return true;
        }

        const char *filename = compressedCrpPath.c_str();

        // Open file
        std::ifstream file;
        file.open(filename, std::ios::binary);
        ASSERT(file.is_open(), "Unable to open CRP " << compressedCrpPath << " for decompression!");

        // Check valid file length
        file.seekg(0, std::ios::end);
        std::streamoff filesize = file.tellg();
        file.seekg(0);
        ASSERT(filesize > 0x10, "CRP at " << compressedCrpPath << " has invalid file size");

        // Initialization
        unsigned int length = 0;
        unsigned char *data = NULL;
        // CRP compression type
        unsigned int id = 0;
        file.read((char *) &id, 4);
        // Uncompressed CRP
        if ((id & 0x0000FFFF) != 0xFB10) {
            file.close();
            // LOG(INFO) << "CRP is already decompressed, skipping";
            std::filesystem::copy_file(compressedCrpPath, decompressedCrpPath, std::filesystem::copy_options::overwrite_existing);
            return true;
        }
        // Compressed CRP
        else {
            // Create uncompressed data array
            file.seekg(2);
            unsigned int elhi, elmd, ello;
            elhi = elmd = ello = 0;
            file.read((char *) &elhi, 1);
            file.read((char *) &elmd, 1);
            file.read((char *) &ello, 1);
            length = (((elhi * 256) + elmd) * 256) + ello;
            data   = new unsigned char[length];
            // Memory allocation
            ASSERT(data != NULL, "Unable to allocate buffer for decompressed CRP data! Possible invalid length read");
            // Initialization
            file.seekg(5);
            unsigned int datapos, len, offset, inbyte, tmp1, tmp2, tmp3;
            unsigned char *srcpos, *dstpos;
            datapos = len = offset = inbyte = tmp1 = tmp2 = tmp3 = 0;
            file.read((char *) &inbyte, 1);
            // Decompress
            while ((!file.eof()) && (inbyte < 0xFC)) {
                if (!(inbyte & 0x80)) {
                    file.read((char *) &tmp1, 1);
                    len = inbyte & 0x03;
                    if (len != 0) {
                        file.read((char *) (data + datapos), len);
                        datapos += len;
                    }
                    len = ((inbyte & 0x1C) >> 2) + 3;
                    if (len != 0) {
                        offset = ((inbyte >> 5) << 8) + tmp1 + 1;
                        dstpos = data + datapos;
                        srcpos = data + datapos - offset;
                        datapos += len;
                        while (len--)
                            *dstpos++ = *srcpos++;
                    }
                } else if (!(inbyte & 0x40)) {
                    file.read((char *) &tmp1, 1);
                    file.read((char *) &tmp2, 1);
                    len = (tmp1 >> 6) & 0x03;
                    if (len != 0) {
                        file.read((char *) (data + datapos), len);
                        datapos += len;
                    }
                    len = (inbyte & 0x3F) + 4;
                    if (len != 0) {
                        offset = ((tmp1 & 0x3F) * 256) + tmp2 + 1;
                        srcpos = data + datapos - offset;
                        dstpos = data + datapos;
                        datapos += len;
                        while (len--)
                            *dstpos++ = *srcpos++;
                    }
                } else if (!(inbyte & 0x20)) {
                    file.read((char *) &tmp1, 1);
                    file.read((char *) &tmp2, 1);
                    file.read((char *) &tmp3, 1);
                    len = inbyte & 0x03;
                    if (len != 0) {
                        file.read((char *) (data + datapos), len);
                        datapos += len;
                    }
                    len = (((inbyte >> 2) & 0x03) * 256) + tmp3 + 5;
                    if (len != 0) {
                        offset = ((inbyte & 0x10) << 0x0C) + (tmp1 * 256) + tmp2 + 1;
                        srcpos = data + datapos - offset;
                        dstpos = data + datapos;
                        datapos += len;
                        while (len--)
                            *dstpos++ = *srcpos++;
                    }
                } else {
                    len = ((inbyte & 0x1F) * 4) + 4;
                    if (len != 0) {
                        file.read((char *) (data + datapos), len);
                        datapos += len;
                    }
                }
                inbyte = tmp1 = tmp2 = tmp3 = 0;
                file.read((char *) &inbyte, 1);
            }
            if ((!file.eof()) && (datapos < length)) {
                len = inbyte & 0x03;
                if (len != 0) {
                    file.read((char *) (data + datapos), len);
                }
            }
        }
        // Clean up
        file.close();

        // Write out uncompressed data
        std::ofstream ofile;
        ofile.open(decompressedCrpPath.c_str(), std::ios::binary);
        ASSERT(ofile.is_open(), "Unable to open output CRP at " << decompressedCrpPath << " for write of decompressed data");
        ofile.write((const char *) data, length);
        ofile.close();

        delete[] data;

        return true;
    }

    glm::vec3 CalculateQuadNormal(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p4) {
        glm::vec3 triANormal = CalculateNormal(p1, p2, p3);
        glm::vec3 triBNormal = CalculateNormal(p1, p3, p4);
        return glm::normalize(triANormal + triBNormal);
    }

    glm::vec3 CalculateNormal(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3) {
        glm::vec3 vertexNormal(0, 0, 0);

        glm::vec3 U = p2 - p1;
        glm::vec3 V = p3 - p1;

        vertexNormal.x = (U.y * V.z) - (U.z * V.y);
        vertexNormal.y = (U.z * V.x) - (U.x * V.z);
        vertexNormal.z = (U.x * V.y) - (U.y * V.x);

        return vertexNormal;
    }
} // namespace LibOpenNFS::Utils
