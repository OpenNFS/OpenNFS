#include "ExtraObjectBlock.h"

using namespace LibOpenNFS::NFS2;

template <typename Platform>
ExtraObjectBlock<Platform>::ExtraObjectBlock(std::ifstream &trk, NFSVersion version) {
    this->version = version;
    ASSERT(this->_SerializeIn(trk), "Failed to serialize ExtraObjectBlock from file stream");
}

template <typename Platform>
bool ExtraObjectBlock<Platform>::_SerializeIn(std::ifstream &ifstream) {
    // Read the header
    onfs_check(safe_read(ifstream, recSize));
    onfs_check(safe_read(ifstream, id));
    onfs_check(safe_read(ifstream, nRecords));

    switch (id) {
    case 2: // First xblock always texture table (in COL)
        nTextures = nRecords;
        polyToQfsTexTable.resize(nTextures);
        onfs_check(safe_read(ifstream, polyToQfsTexTable));
        break;
    case 4:
        nNeighbours = nRecords;
        blockNeighbours.resize(nRecords);
        onfs_check(safe_read(ifstream, blockNeighbours));
        break;
    case 5:
        polyTypes.resize(nRecords);
        onfs_check(safe_read(ifstream, polyTypes));
        break;
    case 6:
        medianData.resize(nRecords);
        onfs_check(safe_read(ifstream, medianData));
        break;
    case 7:
    case 18:
    case 19:
        nStructureReferences = nRecords;
        for (uint32_t structureRefIdx = 0; structureRefIdx < nStructureReferences; ++structureRefIdx) {
            structureReferences.push_back(StructureRefBlock(ifstream));
        }
        break;
    case 8: // XBID 8 3D Structure data: This block is only present if nExtraBlocks != 2 (COL)
        nStructures = nRecords;
        for (uint32_t structureIdx = 0; structureIdx < nStructures; ++structureIdx) {
            structures.push_back(StructureBlock<Platform>(ifstream));
        }
        break;
    case 9:
        nLanes = nRecords;
        laneData.resize(nLanes);
        onfs_check(safe_read(ifstream, laneData));
        break;
    // case 10: // PS1 Specific id, Misc purpose
    // {
    // std::cout << "id 10 NStruct: " << nRecords << std::endl;
    // PS1::TRKBLOCK *ps1TrackBlock = ((PS1::TRKBLOCK *) trackblock);
    // ps1TrackBlock->nUnknownVerts = nRecords;
    // uint8_t xbidHeader[8];
    // trk.read((char *) xbidHeader, 8);
    // for (int i = 0; i < 8; ++i)
    // {
    //     std::cout << (int) xbidHeader[i] << std::endl;
    // }
    // // TODO: Likely these are not VERTS, and the act of adding the parent block center gives meaning where none is
    // present.ps1TrackBlock->unknownVerts = new PS1::VERT[nRecords];
    // for (uint32_t record_Idx = 0; record_Idx < nRecords; ++record_Idx)
    // {
    //     trk.read((char *) &ps1TrackBlock->unknownVerts[record_Idx], sizeof(PS1::VERT));
    // }
    // }
    // break;
    case 13:
        nVroad = nRecords;
        if (this->version == NFSVersion::NFS_2_PS1) {
            ps1VroadData.resize(nVroad);
            onfs_check(safe_read(ifstream, ps1VroadData));
        } else {
            vroadData.resize(nVroad);
            onfs_check(safe_read(ifstream, vroadData));
        }
        break;
    case 15:
        nCollisionData = nRecords;
        collisionData.resize(nCollisionData);
        onfs_check(safe_read(ifstream, collisionData));
        break;
    default:
        //LOG(WARNING) << "Unknown XBID: " << id << " nRecords: " << nRecords << " RecSize: " << recSize;
        break;
    }
    return true;
}

template <typename Platform>
void ExtraObjectBlock<Platform>::_SerializeOut(std::ofstream &ofstream) {
    ASSERT(false, "ExtraObjectBlock output serialization is not currently implemented");
}

template class LibOpenNFS::NFS2::ExtraObjectBlock<PS1>;
template class LibOpenNFS::NFS2::ExtraObjectBlock<PC>;
