#pragma once

#include "../../Common/IRawData.h"

class GeoFile : IRawData
{
public:
    GeoFile() = default;

    static bool Load(const std::string &geoPath, GeoFile &geoFile);
    static void Save(const std::string &geoPath, GeoFile &geoFile);

private:
    bool _SerializeIn(std::ifstream &ifstream) override;
    void _SerializeOut(std::ofstream &ofstream) override;
};