#include "ColFile.h"

using namespace LibOpenNFS::NFS2;

bool ColFile::Load(const std::string &colPath, ColFile &colFile)
{
    LOG(INFO) << "Loading COL File located at " << colPath;
    std::ifstream col(colPath, std::ios::in | std::ios::binary);

    bool loadStatus = colFile._SerializeIn(col);
    col.close();

    return loadStatus;
}

void ColFile::Save(const std::string &colPath, ColFile &colFile)
{
    LOG(INFO) << "Saving COL File to " << colPath;
    std::ofstream col(colPath, std::ios::out | std::ios::binary);
    colFile._SerializeOut(col);
}

bool ColFile::_SerializeIn(std::ifstream &ifstream)
{
    return true;
}

void ColFile::_SerializeOut(std::ofstream &ofstream)
{
    ASSERT(false, "COL output serialization is not currently implemented");
}
