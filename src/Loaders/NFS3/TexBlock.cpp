#include "TexBlock.h"

TexBlock::TexBlock(std::ifstream &frd)
{
    ASSERT(this->_SerializeIn(frd), "Failed to serialize TextureBlock from file stream");
}

bool TexBlock::_SerializeIn(std::ifstream &frd)
{
    SAFE_READ(frd, &width, (sizeof(uint16_t)));
    SAFE_READ(frd, &height, (sizeof(uint16_t)));
    SAFE_READ(frd, &unknown1, (sizeof(uint32_t)));
    SAFE_READ(frd, &corners, sizeof(float) * 8);
    SAFE_READ(frd, &unknown2, (sizeof(uint32_t)));
    SAFE_READ(frd, &isLane, (sizeof(bool)));

    return true;
}

void TexBlock::SerializeOut(std::ofstream &frd)
{
    // TODO: Do I need to align here?
    frd.write((char *) &width, sizeof(uint16_t));
    frd.write((char *) &height, sizeof(uint16_t));
    frd.write((char *) &unknown1, sizeof(uint32_t));
    frd.write((char *) &corners, sizeof(float) * 8);
    frd.write((char *) &unknown2, sizeof(uint32_t));
    frd.write((char *) &isLane, sizeof(bool));
}