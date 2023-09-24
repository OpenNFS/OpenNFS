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
    SAFE_READ(ifstream, &recSize, sizeof(uint32_t));
    SAFE_READ(ifstream, &id, sizeof(uint16_t));
    SAFE_READ(ifstream, &nRecords, sizeof(uint16_t));

    switch (id) {
    case 2: // First xblock always texture table (in COL)
        nTextures = nRecords;
        polyToQfsTexTable.resize(nTextures);
        SAFE_READ(ifstream, polyToQfsTexTable.data(), nTextures * sizeof(TEXTURE_BLOCK));
        break;
    case 4:
        nNeighbours = nRecords;
        blockNeighbours.resize(nRecords);
        SAFE_READ(ifstream, blockNeighbours.data(), nRecords * sizeof(uint16_t));
        break;
    case 5:
        polyTypes.resize(nRecords);
        SAFE_READ(ifstream, polyTypes.data(), nRecords * sizeof(POLY_TYPE));
        break;
    case 6:
        medianData.resize(nRecords);
        SAFE_READ(ifstream, medianData.data(), nRecords * sizeof(MEDIAN_BLOCK));
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
        laneData.resize(nRecords);
        SAFE_READ(ifstream, laneData.data(), nLanes * sizeof(LANE_BLOCK));
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
            SAFE_READ(ifstream, ps1VroadData.data(), nVroad * sizeof(VROAD_VEC));
        } else {
            vroadData.resize(nVroad);
            SAFE_READ(ifstream, vroadData.data(), nVroad * sizeof(VROAD));
        }
        break;
    case 15:
        nCollisionData = nRecords;
        collisionData.resize(nCollisionData);
        SAFE_READ(ifstream, collisionData.data(), nCollisionData * sizeof(COLLISION_BLOCK));
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
