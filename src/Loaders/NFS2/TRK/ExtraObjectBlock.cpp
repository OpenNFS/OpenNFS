#include "ExtraObjectBlock.h"

using namespace LibOpenNFS::NFS2;

template <typename Platform>
ExtraObjectBlock<Platform>::ExtraObjectBlock(std::ifstream &trk)
{
    ASSERT(this->_SerializeIn(trk), "Failed to serialize ExtraObjectBlock from file stream");
}

template <typename Platform>
bool ExtraObjectBlock<Platform>::_SerializeIn(std::ifstream &ifstream)
{
    // Read the header
    SAFE_READ(ifstream, &recSize, sizeof(uint32_t));
    SAFE_READ(ifstream, &id, sizeof(uint16_t));
    SAFE_READ(ifstream, &nRecords, sizeof(uint16_t));

    switch (id)
    {
    case 5:
        polyTypes.reserve(nRecords);
        SAFE_READ(ifstream, polyTypes.data(), nRecords * sizeof(POLY_TYPE));
        break;
    case 4:
        nNeighbours = nRecords;
        blockNeighbours.reserve(nRecords);
        SAFE_READ(ifstream, blockNeighbours.data(), nRecords * sizeof(uint16_t));
        break;
    case 8:
        structures.reserve(nRecords); //, sizeof(typename Platform::GEOM_BLOCK)));
        nStructures = nRecords;
        for (uint32_t structure_Idx = 0; structure_Idx < nStructures; ++structure_Idx)
        {
            // TODO: Serialize each structure into own Class
            std::streamoff padCheck = ifstream.tellg();
            /*trk.read((char *) &structures[structure_Idx].recSize, sizeof(uint32_t));
            trk.read((char *) &structures[structure_Idx].nVerts, sizeof(uint16_t));
            trk.read((char *) &structures[structure_Idx].nPoly, sizeof(uint16_t));
            structures[structure_Idx].vertexTable = static_cast<typename Platform::VERT *>(calloc(structures[structure_Idx].nVerts, sizeof(typename Platform::VERT)));
            for (uint32_t vert_Idx = 0; vert_Idx < structures[structure_Idx].nVerts; ++vert_Idx)
            {
                trk.read((char *) &structures[structure_Idx].vertexTable[vert_Idx], sizeof(typename Platform::VERT));
            }
            structures[structure_Idx].polygonTable = static_cast<typename Platform::POLYGONDATA *>(calloc(structures[structure_Idx].nPoly, sizeof(typename Platform::POLYGONDATA)));
            for (uint32_t poly_Idx = 0; poly_Idx < structures[structure_Idx].nPoly; ++poly_Idx)
            {
                trk.read((char *) &structures[structure_Idx].polygonTable[poly_Idx], sizeof(typename Platform::POLYGONDATA));
            }*/
            ifstream.seekg(structures[structure_Idx].recSize - (ifstream.tellg() - padCheck), std::ios_base::cur); // Eat possible padding
        }
        break;
    case 7:
    case 18:
    case 19:
        nStructureReferences += nRecords;
        for (uint32_t structureRef_Idx = 0; structureRef_Idx < nStructureReferences; ++structureRef_Idx)
        {
            std::streamoff padCheck  = ifstream.tellg();
            StructureBlock structure = StructureBlock(ifstream);
            structureBlocks.push_back(structure);
            ifstream.seekg(structure.recSize - (ifstream.tellg() - padCheck), std::ios_base::cur); // Eat possible padding
        }
        break;
        // PS1 Specific id, Misc purpose
        /*case 10: {
            std::cout << "id 10 NStruct: " << nRecords << std::endl;
            PS1::TRKBLOCK *ps1TrackBlock = ((PS1::TRKBLOCK *) trackblock);
            ps1TrackBlock->nUnknownVerts =  nRecords;
            uint8_t xbidHeader[8];
            trk.read((char *) xbidHeader, 8);
            for(int i = 0; i < 8; ++i){
                std::cout << (int) xbidHeader[i] << std::endl;
            }
            // TODO: Likely these are not VERTS, and the act of adding the parent block center gives meaning where none is
        present. ps1TrackBlock->unknownVerts = new PS1::VERT[nRecords]; for (uint32_t record_Idx = 0;
        record_Idx < nRecords; ++record_Idx) { trk.read((char *) &ps1TrackBlock->unknownVerts[record_Idx],
        sizeof(PS1::VERT));
            }
        }
            break;*/
    case 6:
        medianData.reserve(nRecords);, sizeof(MEDIAN_BLOCK)));
        SAFE_READ(ifstream, medianData.data(), nRecords * sizeof(MEDIAN_BLOCK));
        break;
    case 13:
        nVroad = nRecords;
        vroadData.reserve(nRecords);
        SAFE_READ(ifstream, vroadData.data(), nVroad * sizeof(typename Platform::VROAD));
        break;
    case 9:
        nLanes = nRecords;
        laneData.reserve(nRecords);
        SAFE_READ(ifstream, laneData.data(), nLanes * sizeof(LANE_BLOCK));
        break;
    default:
        LOG(WARNING) << "Unknown id: " << id << " nRecords: " << nRecords << " RecSize: " << recSize;
        break;
    }

    return true;
}

template <typename Platform>
void ExtraObjectBlock<Platform>::_SerializeOut(std::ofstream &ofstream)
{
    ASSERT(false, "ExtraObjectBlock output serialization is not currently implemented");
}