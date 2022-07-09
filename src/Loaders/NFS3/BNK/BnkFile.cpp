#include "BnkFile.h"

using namespace LibOpenNFS::NFS3;

bool BnkFile::Load(const std::string &bnkPath, BnkFile &bnkFile) {
    LOG(INFO) << "Loading COL File located at " << bnkPath;
    std::ifstream bnk(bnkPath, std::ios::in | std::ios::binary);

    bool loadStatus = bnkFile._SerializeIn(bnk);
    bnk.close();

    return loadStatus;
}

void BnkFile::Save(const std::string &bnkPath, BnkFile &bnkFile) {
    LOG(INFO) << "Saving COL File to " << bnkPath;
    std::ofstream bnk(bnkPath, std::ios::out | std::ios::binary);
    bnkFile._SerializeOut(bnk);
}

bool BnkFile::_SerializeIn(std::ifstream &ifstream) {
    // Read in the BNK file header
    SAFE_READ(ifstream, &header, 4);
    if (memcmp(header, "BNKl", sizeof(char)) != 0) {
        LOG(WARNING) << "Invalid BNK file";
        return false;
    }

    SAFE_READ(ifstream, &version, sizeof(uint16_t));
    SAFE_READ(ifstream, &numSounds, sizeof(uint16_t));
    SAFE_READ(ifstream, &firstSoundOffset, sizeof(uint32_t));

    soundOffsets.resize(numSounds);
    SAFE_READ(ifstream, soundOffsets.data(), numSounds * sizeof(uint32_t));

    return true;
}

void BnkFile::_SerializeOut(std::ofstream &ofstream) {
    ASSERT(false, "COL output serialization is not currently implemented");
}
