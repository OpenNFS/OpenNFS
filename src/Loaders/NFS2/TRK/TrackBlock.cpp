#include "TrackBlock.h"

template <typename Platform>
TrackBlock<Platform>::TrackBlock(std::ifstream &frd)
{
    ASSERT(this->_SerializeIn(frd), "Failed to serialize TrackBlock from file stream");
}

template <typename Platform>
bool TrackBlock<Platform>::_SerializeIn(std::ifstream &ifstream)
{
    // Read Header
    // trk.seekg(superblockOffsets[superBlock_Idx] + blockOffsets[block_Idx], std::ios_base::beg);
    // LOG(DEBUG) << "  Block " << block_Idx + 1 << " of " << superblock->nBlocks << " [" << trackblock->header->blockSerial << "]";
    SAFE_READ(ifstream, header, sizeof(TRKBLOCK_HEADER));

    // Sanity Checks
    if ((header.blockSizeDup) || (header.blockSerial > nBlocks))
    {
        LOG(DEBUG) << "   --- Bad Block";
        return false;
    }

    // Read 3D Data
    trackblock->vertexTable = static_cast<typename Platform::VERT *>(
      calloc(static_cast<size_t>(trackblock->header->nStickToNextVerts + trackblock->header->nHighResVert), sizeof(typename Platform::VERT)));
    for (int32_t vert_Idx = 0; vert_Idx < trackblock->header->nStickToNextVerts + trackblock->header->nHighResVert; ++vert_Idx)
    {
        trk.read((char *) &trackblock->vertexTable[vert_Idx], sizeof(typename Platform::VERT));
    }

    trackblock->polygonTable = static_cast<typename Platform::POLYGONDATA *>(
      calloc(static_cast<size_t>(trackblock->header->nLowResPoly + trackblock->header->nMedResPoly + trackblock->header->nHighResPoly), sizeof(typename Platform::POLYGONDATA)));
    for (int32_t poly_Idx = 0; poly_Idx < (trackblock->header->nLowResPoly + trackblock->header->nMedResPoly + trackblock->header->nHighResPoly); ++poly_Idx)
    {
        trk.read((char *) &trackblock->polygonTable[poly_Idx], sizeof(typename Platform::POLYGONDATA));
    }

    // Read Extrablock data
    trk.seekg(superblockOffsets[superBlock_Idx] + blockOffsets[block_Idx] + 64u + trackblock->header->extraBlockTblOffset, std::ios_base::beg);
    // Get extrablock offsets (relative to beginning of TrackBlock)
    uint32_t *extrablockOffsets = (uint32_t *) calloc(trackblock->header->nExtraBlocks, sizeof(uint32_t));
    trk.read((char *) extrablockOffsets, trackblock->header->nExtraBlocks * sizeof(uint32_t));

    for (uint32_t xblock_Idx = 0; xblock_Idx < trackblock->header->nExtraBlocks; ++xblock_Idx)
    {
        trk.seekg(superblockOffsets[superBlock_Idx] + blockOffsets[block_Idx] + extrablockOffsets[xblock_Idx], std::ios_base::beg);
        auto *xblockHeader = static_cast<EXTRABLOCK_HEADER *>(calloc(1, sizeof(EXTRABLOCK_HEADER)));
        trk.read((char *) xblockHeader, sizeof(EXTRABLOCK_HEADER));

        switch (xblockHeader->XBID)
        {
        case 5:
            trackblock->polyTypes = static_cast<POLY_TYPE *>(calloc(xblockHeader->nRecords, sizeof(POLY_TYPE)));
            trk.read((char *) trackblock->polyTypes, xblockHeader->nRecords * sizeof(POLY_TYPE));
            break;
        case 4:
            trackblock->nNeighbours     = xblockHeader->nRecords;
            trackblock->blockNeighbours = (uint16_t *) calloc(xblockHeader->nRecords, sizeof(uint16_t));
            trk.read((char *) trackblock->blockNeighbours, xblockHeader->nRecords * sizeof(uint16_t));
            break;
        case 8:
            trackblock->structures  = static_cast<typename Platform::GEOM_BLOCK *>(calloc(xblockHeader->nRecords, sizeof(typename Platform::GEOM_BLOCK)));
            trackblock->nStructures = xblockHeader->nRecords;
            for (uint32_t structure_Idx = 0; structure_Idx < trackblock->nStructures; ++structure_Idx)
            {
                std::streamoff padCheck = trk.tellg();
                trk.read((char *) &trackblock->structures[structure_Idx].recSize, sizeof(uint32_t));
                trk.read((char *) &trackblock->structures[structure_Idx].nVerts, sizeof(uint16_t));
                trk.read((char *) &trackblock->structures[structure_Idx].nPoly, sizeof(uint16_t));
                trackblock->structures[structure_Idx].vertexTable =
                  static_cast<typename Platform::VERT *>(calloc(trackblock->structures[structure_Idx].nVerts, sizeof(typename Platform::VERT)));
                for (uint32_t vert_Idx = 0; vert_Idx < trackblock->structures[structure_Idx].nVerts; ++vert_Idx)
                {
                    trk.read((char *) &trackblock->structures[structure_Idx].vertexTable[vert_Idx], sizeof(typename Platform::VERT));
                }
                trackblock->structures[structure_Idx].polygonTable =
                  static_cast<typename Platform::POLYGONDATA *>(calloc(trackblock->structures[structure_Idx].nPoly, sizeof(typename Platform::POLYGONDATA)));
                for (uint32_t poly_Idx = 0; poly_Idx < trackblock->structures[structure_Idx].nPoly; ++poly_Idx)
                {
                    trk.read((char *) &trackblock->structures[structure_Idx].polygonTable[poly_Idx], sizeof(typename Platform::POLYGONDATA));
                }
                trk.seekg(trackblock->structures[structure_Idx].recSize - (trk.tellg() - padCheck),
                          std::ios_base::cur); // Eat possible padding
            }
            break;
        case 7:
        case 18:
        case 19:
            trackblock->nStructureReferences += xblockHeader->nRecords;
            for (uint32_t structureRef_Idx = 0; structureRef_Idx < trackblock->nStructureReferences; ++structureRef_Idx)
            {
                std::streamoff padCheck = trk.tellg();
                GEOM_REF_BLOCK structure;
                trk.read((char *) &structure.recSize, sizeof(uint16_t));
                trk.read((char *) &structure.recType, sizeof(uint8_t));
                trk.read((char *) &structure.structureRef, sizeof(uint8_t));
                // Fixed type
                if (structure.recType == 1)
                {
                    trk.read((char *) &structure.refCoordinates, sizeof(VERT_HIGHP));
                }
                else if (structure.recType == 3)
                { // Animated type
                    trk.read((char *) &structure.animLength, sizeof(uint16_t));
                    trk.read((char *) &structure.unknown, sizeof(uint16_t));
                    structure.animationData = static_cast<ANIM_POS *>(calloc(structure.animLength, sizeof(ANIM_POS)));
                    for (uint32_t animation_Idx = 0; animation_Idx < structure.animLength; ++animation_Idx)
                    {
                        trk.read((char *) &structure.animationData[animation_Idx], sizeof(ANIM_POS));
                    }
                }
                else if (structure.recType == 4)
                {
                    // 4 Component PSX Vert data? TODO: Restructure to allow the 4th component to be read
                    trk.read((char *) &structure.refCoordinates, sizeof(VERT_HIGHP));
                }
                else
                {
                    LOG(DEBUG) << "XBID " << xblockHeader->XBID << " Unknown Structure Reference type: " << (int) structure.recType << " Size: " << (int) structure.recSize
                               << " StructRef: " << (int) structure.structureRef;
                    continue;
                }
                trackblock->structureRefData.emplace_back(structure);
                trk.seekg(structure.recSize - (trk.tellg() - padCheck), std::ios_base::cur); // Eat possible padding
            }
            break;
            // PS1 Specific XBID, Misc purpose
            /*case 10: {
                std::cout << "XBID 10 NStruct: " << xblockHeader->nRecords << std::endl;
                PS1::TRKBLOCK *ps1TrackBlock = ((PS1::TRKBLOCK *) trackblock);
                ps1TrackBlock->nUnknownVerts =  xblockHeader->nRecords;
                uint8_t xbidHeader[8];
                trk.read((char *) xbidHeader, 8);
                for(int i = 0; i < 8; ++i){
                    std::cout << (int) xbidHeader[i] << std::endl;
                }
                // TODO: Likely these are not VERTS, and the act of adding the parent block center gives meaning where none is
            present. ps1TrackBlock->unknownVerts = new PS1::VERT[xblockHeader->nRecords]; for (uint32_t record_Idx = 0;
            record_Idx < xblockHeader->nRecords; ++record_Idx) { trk.read((char *) &ps1TrackBlock->unknownVerts[record_Idx],
            sizeof(PS1::VERT));
                }
            }
                break;*/
        case 6:
            trackblock->medianData = static_cast<MEDIAN_BLOCK *>(calloc(xblockHeader->nRecords, sizeof(MEDIAN_BLOCK)));
            trk.read((char *) trackblock->medianData, xblockHeader->nRecords * sizeof(MEDIAN_BLOCK));
            break;
        case 13:
            trackblock->nVroad    = xblockHeader->nRecords;
            trackblock->vroadData = static_cast<typename Platform::VROAD *>(calloc(xblockHeader->nRecords, sizeof(typename Platform::VROAD)));
            trk.read((char *) trackblock->vroadData, trackblock->nVroad * sizeof(typename Platform::VROAD));
            break;
        case 9:
            trackblock->nLanes   = xblockHeader->nRecords;
            trackblock->laneData = static_cast<LANE_BLOCK *>(calloc(xblockHeader->nRecords, sizeof(LANE_BLOCK)));
            trk.read((char *) trackblock->laneData, trackblock->nLanes * sizeof(LANE_BLOCK));
            break;
        default:
            LOG(WARNING) << "Unknown XBID: " << xblockHeader->XBID << " nRecords: " << xblockHeader->nRecords << " RecSize: " << xblockHeader->recSize;
            break;
        }
    }

    return true;
}

template <typename Platform>
void TrackBlock<Platform>::_SerializeOut(std::ofstream &ofstream)
{
}