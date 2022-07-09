#pragma once

#include "../Common/IRawData.h"

struct CameraAnimPoint {
    glm::ivec3 pt;
    int16_t od1, od2, od3, od4; // OD2 seems to be something more than just rotation, like zoom or some shit
                                // OD3 seems to set perspective
                                // OD4 similar to OD1, induces wavyness but animation remains
};

class CanFile : IRawData {
public:
    CanFile() = default;
    static bool Load(const std::string &canPath, CanFile &canFile);
    static void Save(const std::string &canPath, CanFile &canFile);

    uint16_t size;
    uint8_t type, struct3D;
    uint16_t animLength, unknown;
    std::vector<CameraAnimPoint> animPoints;

private:
    bool _SerializeIn(std::ifstream &ifstream) override;
    void _SerializeOut(std::ofstream &ofstream) override;
};
