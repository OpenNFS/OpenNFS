#include "StructureBlock.h"

using namespace LibOpenNFS::NFS2;

template <typename Platform>
StructureBlock<Platform>::StructureBlock(std::ifstream &ifstream) {
    ASSERT(this->_SerializeIn(ifstream), "Failed to serialize StructureBlock from file stream");
}

template <typename Platform>
bool StructureBlock<Platform>::_SerializeIn(std::ifstream &ifstream) {
    std::streamoff padCheck = ifstream.tellg();

    onfs_check(safe_read(ifstream, recSize));
    onfs_check(safe_read(ifstream, nVerts));
    onfs_check(safe_read(ifstream, nPoly));

    vertexTable.resize(nVerts);
    onfs_check(safe_read(ifstream, vertexTable));

    polygonTable.resize(nPoly);
    onfs_check(safe_read(ifstream, polygonTable));

    ifstream.seekg(recSize - (ifstream.tellg() - padCheck), std::ios_base::cur); // Eat possible padding

    return true;
}

template <typename Platform>
void StructureBlock<Platform>::_SerializeOut(std::ofstream &ofstream) {
    ASSERT(false, "StructureBlock output serialization is not currently implemented");
}

template class LibOpenNFS::NFS2::StructureBlock<PS1>;
template class LibOpenNFS::NFS2::StructureBlock<PC>;