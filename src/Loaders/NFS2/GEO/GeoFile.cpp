#include "GeoFile.h"

bool GeoFile::Load(const std::string &geoPath, GeoFile &geoFile)
{
    LOG(INFO) << "Loading FCE File located at " << geoPath;
    std::ifstream geo(geoPath, std::ios::in | std::ios::binary);

    bool loadStatus = geoFile._SerializeIn(geo);
    geo.close();

    return loadStatus;
}

void GeoFile::Save(const std::string &geoPath, GeoFile &geoFile)
{
    LOG(INFO) << "Saving FCE File to " << geoPath;
    std::ofstream geo(geoPath, std::ios::out | std::ios::binary);
    geoFile._SerializeOut(geo);
}

bool GeoFile::_SerializeIn(std::ifstream &ifstream)
{
    return true;
}

void GeoFile::_SerializeOut(std::ofstream &ofstream)
{
    ASSERT(false, "GEO output serialization is not currently implemented");
}
