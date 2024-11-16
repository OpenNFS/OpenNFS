#include "ExtraObjectBlock.h"

using namespace LibOpenNFS::NFS3;

ExtraObjectBlock::ExtraObjectBlock(std::ifstream &frd) {
    ASSERT(this->_SerializeIn(frd), "Failed to serialize ExtraObjectBlock from file stream");
}

bool ExtraObjectBlock::_SerializeIn(std::ifstream &ifstream) {
    onfs_check(safe_read(ifstream, nobj));
    obj.reserve(nobj);

    for (uint32_t xobjIdx = 0; xobjIdx < nobj; ++xobjIdx) {
        ExtraObjectData x;

        onfs_check(safe_read(ifstream, x.crosstype));
        onfs_check(safe_read(ifstream, x.crossno));
        onfs_check(safe_read(ifstream, x.unknown));

        if (x.crosstype == 4) {
            // Basic objects
            onfs_check(safe_read(ifstream, x.ptRef, sizeof(glm::vec3)));
            onfs_check(safe_read(ifstream, x.AnimMemory));
        } else if (x.crosstype == 3) {
            // Animated objects
            onfs_check(safe_read(ifstream, x.unknown3, sizeof(uint16_t) * 9));
            onfs_check(safe_read(ifstream, x.type3));
            onfs_check(safe_read(ifstream, x.objno));
            onfs_check(safe_read(ifstream, x.nAnimLength));
            onfs_check(safe_read(ifstream, x.AnimDelay));

            // Sanity Check
            if (x.type3 != 3) {
                return false;
            }

            x.animData.resize(x.nAnimLength);
            onfs_check(safe_read(ifstream, x.animData));
            // make a ref point from first anim position
            x.ptRef = Utils::FixedToFloat(x.animData[0].pt);
        } else
            return false; // unknown object type

        // Get number of vertices
        onfs_check(safe_read(ifstream, x.nVertices));

        // Get vertices
        x.vert.resize(x.nVertices);
        onfs_check(safe_read(ifstream, x.vert));

        // Per vertex shading data (RGBA)
        x.vertShading.resize(x.nVertices);
        onfs_check(safe_read(ifstream, x.vertShading));

        // Get number of polygons
        onfs_check(safe_read(ifstream, x.nPolygons));

        // Grab the polygons
        x.polyData.resize(x.nPolygons);
        onfs_check(safe_read(ifstream, x.polyData));

        obj.push_back(x);
    }

    return true;
}

void ExtraObjectBlock::_SerializeOut(std::ofstream &ofstream) {
    ofstream.write((char *) &(nobj), sizeof(uint32_t));

    for (uint32_t xobjIdx = 0; xobjIdx < nobj; ++xobjIdx) {
        ofstream.write((char *) &obj[xobjIdx].crosstype, sizeof(uint32_t));
        ofstream.write((char *) &obj[xobjIdx].crossno, sizeof(uint32_t));
        ofstream.write((char *) &obj[xobjIdx].unknown, sizeof(uint32_t));

        if (obj[xobjIdx].crosstype == 4) {
            // Basic objects
            ofstream.write((char *) &obj[xobjIdx].ptRef, sizeof(glm::vec3));
            ofstream.write((char *) &obj[xobjIdx].AnimMemory, sizeof(uint32_t));
        } else if (obj[xobjIdx].crosstype == 3) {
            // Animated objects
            ofstream.write((char *) &obj[xobjIdx].unknown3, sizeof(uint16_t) * 9);
            ofstream.write((char *) &obj[xobjIdx].type3, sizeof(uint8_t));
            ofstream.write((char *) &obj[xobjIdx].objno, sizeof(uint8_t));
            ofstream.write((char *) &obj[xobjIdx].nAnimLength, sizeof(uint16_t));
            ofstream.write((char *) &obj[xobjIdx].AnimDelay, sizeof(uint16_t));
            ofstream.write((char *) obj[xobjIdx].animData.data(), sizeof(AnimData) * obj[xobjIdx].nAnimLength);
        }
        ofstream.write((char *) &(obj[xobjIdx].nVertices), sizeof(uint32_t));
        ofstream.write((char *) obj[xobjIdx].vert.data(), sizeof(glm::vec3) * obj[xobjIdx].nVertices);
        ofstream.write((char *) obj[xobjIdx].vertShading.data(), sizeof(uint32_t) * obj[xobjIdx].nVertices);
        ofstream.write((char *) &(obj[xobjIdx].nPolygons), sizeof(uint32_t));
        ofstream.write((char *) obj[xobjIdx].polyData.data(), sizeof(PolygonData) * obj[xobjIdx].nPolygons);
    }
}