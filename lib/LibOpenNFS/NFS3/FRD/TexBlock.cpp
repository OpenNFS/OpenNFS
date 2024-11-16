#include "TexBlock.h"

using namespace LibOpenNFS::NFS3;

TexBlock::TexBlock(std::ifstream &frd) {
    ASSERT(this->_SerializeIn(frd), "Failed to serialize TextureBlock from file stream");
}

bool TexBlock::_SerializeIn(std::ifstream &ifstream) {
    onfs_check(safe_read(ifstream, width, (sizeof(uint16_t))));
    onfs_check(safe_read(ifstream, height, (sizeof(uint16_t))));
    onfs_check(safe_read(ifstream, unknown1, (sizeof(uint32_t))));
    onfs_check(safe_read(ifstream, corners, sizeof(float) * 8));
    onfs_check(safe_read(ifstream, unknown2, (sizeof(uint32_t))));
    onfs_check(safe_read(ifstream, isLane, (sizeof(bool))));
    onfs_check(safe_read(ifstream, qfsIndex, (sizeof(uint16_t))));

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
