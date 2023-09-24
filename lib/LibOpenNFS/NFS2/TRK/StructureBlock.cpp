#include "StructureBlock.h"

using namespace LibOpenNFS::NFS2;

template <typename Platform>
StructureBlock<Platform>::StructureBlock(std::ifstream &ifstream) {
    ASSERT(this->_SerializeIn(ifstream), "Failed to serialize StructureBlock from file stream");
}

template <typename Platform>
bool StructureBlock<Platform>::_SerializeIn(std::ifstream &ifstream) {
    std::streamoff padCheck = ifstream.tellg();

    SAFE_READ(ifstream, &recSize, sizeof(uint32_t));
    SAFE_READ(ifstream, &nVerts, sizeof(uint16_t));
    SAFE_READ(ifstream, &nPoly, sizeof(uint16_t));

    vertexTable.resize(nVerts);
    SAFE_READ(ifstream, vertexTable.data(), nVerts * sizeof(typename Platform::VERT));

    polygonTable.resize(nPoly);
    SAFE_READ(ifstream, polygonTable.data(), nPoly * sizeof(typename Platform::POLYGONDATA));

    ifstream.seekg(recSize - (ifstream.tellg() - padCheck), std::ios_base::cur); // Eat possible padding

    return true;
}

template <typename Platform>
void StructureBlock<Platform>::_SerializeOut(std::ofstream &ofstream) {
    ASSERT(false, "StructureBlock output serialization is not currently implemented");
}

template class LibOpenNFS::NFS2::StructureBlock<PS1>;
template class LibOpenNFS::NFS2::StructureBlock<PC>;