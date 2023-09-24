#include "StructureRefBlock.h"

using namespace LibOpenNFS::NFS2;

StructureRefBlock::StructureRefBlock(std::ifstream &trk) {
    ASSERT(this->_SerializeIn(trk), "Failed to serialize StructureRefBlock from file stream");
}

bool StructureRefBlock::_SerializeIn(std::ifstream &ifstream) {
    std::streamoff padCheck = ifstream.tellg();

    SAFE_READ(ifstream, &recSize, sizeof(uint16_t));
    SAFE_READ(ifstream, &recType, sizeof(uint8_t));
    SAFE_READ(ifstream, &structureRef, sizeof(uint8_t));

    if (recType == 1) {
        // Fixed type
        SAFE_READ(ifstream, &refCoordinates, sizeof(VERT_HIGHP));
    } else if (recType == 3) {
        // Animated type
        SAFE_READ(ifstream, &animLength, sizeof(uint16_t));
        SAFE_READ(ifstream, &unknown, sizeof(uint16_t));
        animationData.resize(animLength);
        SAFE_READ(ifstream, animationData.data(), animLength * sizeof(ANIM_POS));
    } else if (recType == 4) {
        // 4 Component PSX Vert data? TODO: Restructure to allow the 4th component to be read
        SAFE_READ(ifstream, &refCoordinates, sizeof(VERT_HIGHP));
    } else {
        //LOG(DEBUG) << "Unknown Structure Reference type: " << (int) recType << " Size: " << (int) recSize << " StructRef: " << (int) structureRef;
        return true;
    }

    ifstream.seekg(recSize - (ifstream.tellg() - padCheck), std::ios_base::cur); // Eat possible padding

    return true;
}

void StructureRefBlock::_SerializeOut(std::ofstream &ofstream) {
    ASSERT(false, "StructureRefBlock output serialization is not currently implemented");
}
