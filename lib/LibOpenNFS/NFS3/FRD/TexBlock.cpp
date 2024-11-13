#include "TexBlock.h"

using namespace LibOpenNFS::NFS3;

TexBlock::TexBlock(std::ifstream &frd) {
    ASSERT(this->_SerializeIn(frd), "Failed to serialize TextureBlock from file stream");
}

bool TexBlock::_SerializeIn(std::ifstream &ifstream) {
    SAFE_READ(ifstream, &width, (sizeof(uint16_t)));
    SAFE_READ(ifstream, &height, (sizeof(uint16_t)));
    SAFE_READ(ifstream, &unknown1, (sizeof(uint32_t)));
    SAFE_READ(ifstream, &corners, sizeof(float) * 8);
    SAFE_READ(ifstream, &unknown2, (sizeof(uint32_t)));
    SAFE_READ(ifstream, &isLane, (sizeof(bool)));
    SAFE_READ(ifstream, &qfsIndex, (sizeof(uint16_t)));

    return true;
}

void TexBlock::_SerializeOut(std::ofstream &ofstream) {
    // TODO: Do I need to align here?
    ofstream.write((char *) &width, sizeof(uint16_t));
    ofstream.write((char *) &height, sizeof(uint16_t));
    ofstream.write((char *) &unknown1, sizeof(uint32_t));
    ofstream.write((char *) &corners, sizeof(float) * 8);
    ofstream.write((char *) &unknown2, sizeof(uint32_t));
    ofstream.write((char *) &isLane, sizeof(bool));
    ofstream.write((char *) &qfsIndex, sizeof(uint16_t));
}

std::vector<glm::vec2> TexBlock::GenerateUVs(bool is_xobj) {
    std::vector<glm::vec2> uvs;

    if (is_xobj) {
        uvs.emplace_back((1.0f - corners[0]), (1.0f - corners[1]));
        uvs.emplace_back((1.0f - corners[2]), (1.0f - corners[3]));
        uvs.emplace_back((1.0f - corners[4]), (1.0f - corners[5]));
        uvs.emplace_back((1.0f - corners[0]), (1.0f - corners[1]));
        uvs.emplace_back((1.0f - corners[4]), (1.0f - corners[5]));
        uvs.emplace_back((1.0f - corners[6]), (1.0f - corners[7]));
    } else {
        // OBJ_POLY, LANE, ROAD
        uvs.emplace_back(corners[0], (1.0f - corners[1]));
        uvs.emplace_back(corners[2], (1.0f - corners[3]));
        uvs.emplace_back(corners[4], (1.0f - corners[5]));
        uvs.emplace_back(corners[0], (1.0f - corners[1]));
        uvs.emplace_back(corners[4], (1.0f - corners[5]));
        uvs.emplace_back(corners[6], (1.0f - corners[7]));
    }
    return uvs;
}