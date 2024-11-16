#include "CanFile.h"

bool CanFile::Load(const std::string &canPath, CanFile &canFile) {
    //LOG(INFO) << "Loading CAN File located at " << canPath;
    std::ifstream can(canPath, std::ios::in | std::ios::binary);

    bool loadStatus = canFile._SerializeIn(can);
    can.close();

    return loadStatus;
}

void CanFile::Save(const std::string &canPath, CanFile &canFile) {
    //LOG(INFO) << "Saving CAN File to " << canPath;
    std::ofstream can(canPath, std::ios::out | std::ios::binary);
    canFile._SerializeOut(can);
}

bool CanFile::_SerializeIn(std::ifstream &ifstream) {
    onfs_check(safe_read(ifstream, size));
    onfs_check(safe_read(ifstream, type));
    onfs_check(safe_read(ifstream, struct3D));
    onfs_check(safe_read(ifstream, animLength));
    onfs_check(safe_read(ifstream, unknown));

    animPoints.resize(animLength);
    onfs_check(safe_read(ifstream, animPoints));

    return true;
}

void CanFile::_SerializeOut(std::ofstream &ofstream) {
    ofstream.write((char *) &size, sizeof(uint16_t));
    ofstream.write((char *) &type, sizeof(uint8_t));
    ofstream.write((char *) &struct3D, sizeof(uint8_t));
    ofstream.write((char *) &animLength, sizeof(uint16_t));
    ofstream.write((char *) &unknown, sizeof(uint16_t));
    ofstream.write((char *) animPoints.data(), sizeof(CameraAnimPoint) * animLength);
    ofstream.close();
}
