#include "BnkFile.h"

#include <cstring>

using namespace LibOpenNFS::NFS3;

bool BnkFile::Load(const std::string &bnkPath, BnkFile &bnkFile) {
    //LOG(INFO) << "Loading COL File located at " << bnkPath;
    std::ifstream bnk(bnkPath, std::ios::in | std::ios::binary);

    bool loadStatus = bnkFile._SerializeIn(bnk);
    bnk.close();

    return loadStatus;
}

void BnkFile::Save(const std::string &bnkPath, BnkFile &bnkFile) {
    //LOG(INFO) << "Saving COL File to " << bnkPath;
    std::ofstream bnk(bnkPath, std::ios::out | std::ios::binary);
    bnkFile._SerializeOut(bnk);
}

bool BnkFile::_SerializeIn(std::ifstream &ifstream) {
    // Read in the BNK file header
    onfs_check(safe_read(ifstream, header, 4));
    if (memcmp(header, "BNKl", sizeof(char)) != 0) {
        //LOG(WARNING) << "Invalid BNK file";
        return false;
    }

    onfs_check(safe_read(ifstream, &version));
    onfs_check(safe_read(ifstream, &numSounds));
    onfs_check(safe_read(ifstream, &firstSoundOffset));

    soundOffsets.resize(numSounds);
    onfs_check(safe_read(ifstream, soundOffsets);

    return true;
}

void BnkFile::_SerializeOut(std::ofstream &ofstream) {
    ASSERT(false, "COL output serialization is not currently implemented");
}
