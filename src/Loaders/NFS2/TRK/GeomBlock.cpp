#include "GeomBlock.h"

using namespace LibOpenNFS::NFS2;

template class LibOpenNFS::NFS2::GeomBlock<PS1>;
template class LibOpenNFS::NFS2::GeomBlock<PC>;

template <typename Platform>
GeomBlock<Platform>::GeomBlock(std::ifstream &ifstream)
{
    ASSERT(this->_SerializeIn(ifstream), "Failed to serialize GeomBlock from file stream");
}

template <typename Platform>
bool GeomBlock<Platform>::_SerializeIn(std::ifstream &ifstream)
{
    std::streamoff padCheck = ifstream.tellg();

    SAFE_READ(ifstream, &recSize, sizeof(uint32_t));
    SAFE_READ(ifstream, &nVerts, sizeof(uint16_t));
    SAFE_READ(ifstream, &nPoly, sizeof(uint16_t));

    vertexTable.reserve(nVerts);
    SAFE_READ(ifstream, vertexTable.data(), nVerts * sizeof(typename Platform::VERT));

    polygonTable.reserve(nPoly);
    SAFE_READ(ifstream, polygonTable.data(), nPoly * sizeof(typename Platform::POLYGONDATA));

    ifstream.seekg(recSize - (ifstream.tellg() - padCheck), std::ios_base::cur); // Eat possible padding

    return true;
}

template <typename Platform>
void GeomBlock<Platform>::_SerializeOut(std::ofstream &ofstream)
{
    ASSERT(false, "GeomBlock output serialization is not currently implemented");
}
