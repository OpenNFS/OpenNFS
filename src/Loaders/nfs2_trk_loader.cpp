//
// Created by Amrik on 24/05/2018.
//

#include <iostream>
#include <sstream>
#include <boost/filesystem/path.hpp>
#include "nfs2_trk_loader.h"

using namespace std;

namespace NFS2{
    TRACK *trk_loadera(const std::string &track_base_path) {
        std::cout << "--- Loading NFS2 Track ---" << std::endl;

        TRACK *track = static_cast<TRACK *>(calloc(1, sizeof(TRACK)));
        boost::filesystem::path p(track_base_path);
        std::string track_name = p.filename().string();
        stringstream trk_path, col_path;

        trk_path << track_base_path <<  "/" << track_name << ".TRK";
        col_path << track_base_path <<  "/" << track_name << ".col";
        //"/mnt/c/Users/Amrik/Development/OpenNFS3/resources/NFS2/tr00/TR00.TRK"
        if(LoadTRK(trk_path.str(), track)){
            LoadCOL(col_path.str(), track); // Load Catalogue file to get global (non trkblock specific) data
        };

        return track;
    }

    bool LoadTRK(std::string trk_path, TRACK *track) {
        std::cout << "- Parsing TRK File " << std::endl;
        ifstream trk(trk_path, ios::in | ios::binary);
        // TRK file header data
        unsigned char header[4];
        long unknownHeader[5];

        // Check we're in a valid TRK file
        if (trk.read(((char *) header), sizeof(unsigned char) * 4).gcount() != sizeof(unsigned char) * 4) {
            std::cout << trk_path << std::endl;
            return false;
        }
        // Header should contain TRAC
        if (header[0] != 'T' || header[3] != 'C'){
            std::cout << "Invalid TRK file." << std::endl;
            return false;
        }

        // Unknown header data
        if (trk.read(((char *) unknownHeader), sizeof(uint32_t) * 5).gcount() != sizeof(uint32_t) * 5) return false;

        // Basic Track data
        trk.read((char *) &track->nSuperBlocks, sizeof(uint32_t));
        trk.read((char *) &track->nBlocks, sizeof(uint32_t));
        track->superblocks = static_cast<SUPERBLOCK *>(calloc(track->nBlocks, sizeof(SUPERBLOCK)));

        // Offsets of Superblocks in TRK file
        uint32_t superblockOffsets[track->nSuperBlocks];
        if (trk.read(((char *) superblockOffsets), track->nSuperBlocks * sizeof(uint32_t)).gcount() !=
            track->nSuperBlocks * sizeof(uint32_t))
            return false;

        // Reference coordinates for each block
        VERT_HIGH *blockReferenceCoords = static_cast<VERT_HIGH *>(calloc(track->nBlocks, sizeof(VERT_HIGH)));
        if (trk.read((char *) blockReferenceCoords, track->nBlocks * sizeof(VERT_HIGH)).gcount() != track->nBlocks * sizeof(VERT_HIGH)){
            free(blockReferenceCoords);
            return false;
        }

        for (int superBlock_Idx = 0; superBlock_Idx < track->nSuperBlocks; ++superBlock_Idx) {
            std::cout << "SuperBlock " << superBlock_Idx+1 << " of " << track->nSuperBlocks << std::endl;
            // Get the superblock header
            SUPERBLOCK *superblock = &track->superblocks[superBlock_Idx];
            trk.seekg(superblockOffsets[superBlock_Idx], ios_base::beg);
            trk.read((char *) &superblock->superBlockSize, sizeof(uint32_t));
            trk.read((char *) &superblock->nBlocks, sizeof(uint32_t));
            trk.read((char *) &superblock->padding, sizeof(uint32_t));

            if (superblock->nBlocks != 0) {
                // Get the offsets of the child blocks within superblock
                uint32_t *blockOffsets = (uint32_t *) calloc(static_cast<size_t>(superblock->nBlocks), sizeof(uint32_t));
                trk.read((char *) blockOffsets, superblock->nBlocks * sizeof(uint32_t));
                superblock->trackBlocks = static_cast<TRKBLOCK *>(calloc(static_cast<size_t>(superblock->nBlocks), sizeof(TRKBLOCK)));

                for (int block_Idx = 0; block_Idx < superblock->nBlocks; ++block_Idx) {
                    std::cout << "  Block " << block_Idx+1 << " of " << superblock->nBlocks << std::endl;
                    TRKBLOCK *trackblock = &superblock->trackBlocks[block_Idx];
                    // Read Header
                    trackblock->header = static_cast<TRKBLOCK_HEADER *>(calloc(1, sizeof(TRKBLOCK_HEADER)));
                    trk.seekg(superblockOffsets[superBlock_Idx] + blockOffsets[block_Idx], ios_base::beg);
                    trk.read((char *) trackblock->header, sizeof(TRKBLOCK_HEADER));

                    // Sanity Checks
                    if((trackblock->header->blockSize != trackblock->header->blockSizeDup)||(trackblock->header->blockSerial > track->nBlocks)){
                        std::cout<< "   --- Bad Block" << std::endl;
                        free(blockReferenceCoords);
                        return false;
                    }

                    // Read 3D Data
                    trackblock->vertexTable = (VERT *) calloc(static_cast<size_t>(trackblock->header->nStickToNextVerts + trackblock->header->nHighResVert), sizeof(VERT));
                    for(int vert_Idx = 0; vert_Idx < trackblock->header->nStickToNextVerts + trackblock->header->nHighResVert; ++vert_Idx){
                        trk.read((char *) &trackblock->vertexTable[vert_Idx], sizeof(VERT));
                    }

                    trackblock->polygonTable =static_cast<POLYGONDATA *>(calloc(static_cast<size_t>(trackblock->header->nLowResPoly + trackblock->header->nMedResPoly + trackblock->header->nHighResPoly), sizeof(POLYGONDATA)));
                    for(unsigned int poly_Idx = 0; poly_Idx < (trackblock->header->nLowResPoly + trackblock->header->nMedResPoly + trackblock->header->nHighResPoly); ++poly_Idx){
                        trk.read((char *) &trackblock->polygonTable[poly_Idx], sizeof(POLYGONDATA));
                    }

                    // Read Extrablock data
                    trk.seekg(superblockOffsets[superBlock_Idx] + blockOffsets[block_Idx] + 64u + trackblock->header->extraBlockTblOffset, ios_base::beg);
                    // Get extrablock offsets (relative to beginning of TrackBlock)
                    uint32_t *extrablockOffsets = (uint32_t *) calloc(trackblock->header->nExtraBlocks, sizeof(uint32_t));
                    trk.read((char *) extrablockOffsets, trackblock->header->nExtraBlocks * sizeof(uint32_t));

                    for(int xblock_Idx = 0; xblock_Idx < trackblock->header->nExtraBlocks; ++xblock_Idx){
                        trk.seekg(superblockOffsets[superBlock_Idx] + blockOffsets[block_Idx] + extrablockOffsets[xblock_Idx], ios_base::beg);
                        EXTRABLOCK_HEADER *xblockHeader = static_cast<EXTRABLOCK_HEADER *>(calloc(1, sizeof(EXTRABLOCK_HEADER)));
                        trk.read((char*) xblockHeader, sizeof(EXTRABLOCK_HEADER));
                        switch(xblockHeader->XBID){
                            case 5:
                                trackblock->polyTypes = static_cast<POLY_TYPE *>(calloc(xblockHeader->nRecords, sizeof(POLY_TYPE)));
                                trk.read((char *) trackblock->polyTypes, xblockHeader->nRecords * sizeof(POLY_TYPE));
                                break;
                            case 4:
                                trackblock->nNeighbours = xblockHeader->nRecords;
                                trackblock->blockNeighbours = (uint16_t *) calloc(xblockHeader->nRecords, sizeof(uint16_t));
                                trk.read((char*) trackblock->blockNeighbours, xblockHeader->nRecords * sizeof(uint16_t));
                                break;
                            case 8:
                                trackblock->structures = static_cast<GEOM_BLOCK *>(calloc(xblockHeader->nRecords, sizeof(GEOM_BLOCK)));
                                trackblock->nStructures = xblockHeader->nRecords;
                                for(int structure_Idx = 0; structure_Idx < trackblock->nStructures; ++structure_Idx){
                                    streamoff padCheck = trk.tellg();
                                    trk.read((char*) &trackblock->structures[structure_Idx].recSize, sizeof(uint32_t));
                                    trk.read((char*) &trackblock->structures[structure_Idx].nVerts, sizeof(uint16_t));
                                    trk.read((char*) &trackblock->structures[structure_Idx].nPoly, sizeof(uint16_t));
                                    trackblock->structures[structure_Idx].vertexTable = (VERT *) calloc(trackblock->structures[structure_Idx].nVerts, sizeof(VERT));
                                    for(int vert_Idx = 0; vert_Idx < trackblock->structures[structure_Idx].nVerts; ++vert_Idx){
                                        trk.read((char *) &trackblock->structures[structure_Idx].vertexTable[vert_Idx], sizeof(VERT));
                                    }
                                    trackblock->structures[structure_Idx].polygonTable = static_cast<POLYGONDATA *>(calloc(trackblock->structures[structure_Idx].nPoly, sizeof(POLYGONDATA)));
                                    for(int poly_Idx = 0; poly_Idx < trackblock->structures[structure_Idx].nPoly; ++poly_Idx){
                                        trk.read((char *) &trackblock->structures[structure_Idx].polygonTable[poly_Idx], sizeof(POLYGONDATA));
                                    }
                                    trk.seekg(trackblock->structures[structure_Idx].recSize - (trk.tellg() - padCheck), ios_base::cur); // Eat possible padding
                                }
                                break;
                            case 7:
                            case 18:
                                trackblock->structureRefData = static_cast<GEOM_REF_BLOCK *>(calloc(xblockHeader->nRecords, sizeof(GEOM_REF_BLOCK)));
                                trackblock->nStructureReferences = xblockHeader->nRecords;
                                for(int structureRef_Idx = 0; structureRef_Idx < trackblock->nStructureReferences; ++structureRef_Idx){
                                    streamoff padCheck = trk.tellg();
                                    trk.read((char*) &trackblock->structureRefData[structureRef_Idx].recSize, sizeof(uint16_t));
                                    trk.read((char*) &trackblock->structureRefData[structureRef_Idx].recType, sizeof(uint8_t));
                                    trk.read((char*) &trackblock->structureRefData[structureRef_Idx].structureRef, sizeof(uint8_t));
                                    // Fixed type
                                    if(trackblock->structureRefData[structureRef_Idx].recType == 1){
                                        trk.read((char*) &trackblock->structureRefData[structureRef_Idx].refCoordinates, sizeof(VERT_HIGH));
                                    }
                                    else if(trackblock->structureRefData[structureRef_Idx].recType == 3){ // Animated type
                                        trk.read((char*) &trackblock->structureRefData[structureRef_Idx].animLength, sizeof(uint16_t));
                                        trk.read((char*) &trackblock->structureRefData[structureRef_Idx].unknown, sizeof(uint16_t));
                                        trackblock->structureRefData[structureRef_Idx].animationData = static_cast<ANIM_POS *>(calloc(trackblock->structureRefData[structureRef_Idx].animLength, sizeof(ANIM_POS)));
                                        for(int animation_Idx = 0; animation_Idx < trackblock->structureRefData[structureRef_Idx].animLength; ++animation_Idx){
                                            trk.read((char*) &trackblock->structureRefData[structureRef_Idx].animationData[animation_Idx], sizeof(ANIM_POS));
                                        }
                                    } else {
                                        std::cout << "Unknown Structure Reference type: " << (int) trackblock->structureRefData[structureRef_Idx].recType << std::endl;
                                    }
                                    trk.seekg(trackblock->structureRefData[structureRef_Idx].recSize - (trk.tellg() - padCheck), ios_base::cur); // Eat possible padding
                                }
                                break;
                            case 6:
                                trackblock->medianData = static_cast<MEDIAN_BLOCK *>(calloc(xblockHeader->nRecords, sizeof(MEDIAN_BLOCK)));
                                trk.read((char *) trackblock->medianData, xblockHeader->nRecords * sizeof(MEDIAN_BLOCK));
                                break;
                            case 13:
                                trackblock->nVroad = xblockHeader->nRecords;
                                trackblock->vroadData = static_cast<VROAD *>(calloc(xblockHeader->nRecords, sizeof(VROAD)));
                                trk.read((char *) trackblock->vroadData, trackblock->nVroad * sizeof(VROAD));
                                break;
                            case 9:
                                trackblock->nLanes = xblockHeader->nRecords;
                                trackblock->laneData = static_cast<LANE_BLOCK *>(calloc(xblockHeader->nRecords, sizeof(LANE_BLOCK)));
                                trk.read((char *) trackblock->laneData, trackblock->nLanes * sizeof(LANE_BLOCK));
                                break;
                            default:
                                std::cout << "Unknown XBID: " << xblockHeader->XBID << std::endl;
                                break;
                        }
                        free(xblockHeader);
                    }
                    free(extrablockOffsets);
                }
                free(blockOffsets);
            }
        }
        dbgPrintVerts(track, blockReferenceCoords, "C:/Users/Amrik/Desktop/Tr02b/", true);
        free(blockReferenceCoords);
        trk.close();
        return true;
    }

    bool LoadCOL(std::string col_path, TRACK *track) {
        std::cout << "- Parsing COL File " << std::endl;
        ifstream col(col_path, ios::in | ios::binary);
        // Check we're in a valid TRK file
        unsigned char header[4];
        if (col.read(((char *) header), sizeof(unsigned char) * 4).gcount() != sizeof(unsigned char) * 4) return false;
        if (!strcmp(reinterpret_cast<const char *>(header), "COLL")) return false;

        uint32_t version;
        col.read((char *) &version, sizeof(uint32_t));
        if (version != 11) return false;

        uint32_t colSize;
        col.read((char *) &colSize, sizeof(uint32_t));

        uint32_t nExtraBlocks;
        col.read((char*) &nExtraBlocks, sizeof(uint32_t));

        uint32_t *extraBlockOffsets = (uint32_t *) calloc(nExtraBlocks, sizeof(uint32_t));
        col.read((char*) extraBlockOffsets, nExtraBlocks*sizeof(uint32_t));

        for(int xBlock_Idx = 0; xBlock_Idx < nExtraBlocks; ++xBlock_Idx) {
            col.seekg(16 + extraBlockOffsets[xBlock_Idx] + 16, ios_base::beg);

            // TODO: Fix reading of all other blocks that aren't TexTable
            switch (xBlock_Idx) {
                case 0: // First xbock always texture table
                    track->nTextures = (extraBlockOffsets[1] - extraBlockOffsets[0])/sizeof(TEXTURE_BLOCK);
                    track->polyToQFStexTable = static_cast<TEXTURE_BLOCK *>(calloc(track->nTextures, sizeof(TEXTURE_BLOCK)));
                    col.read((char *) track->polyToQFStexTable, track->nTextures * sizeof(TEXTURE_BLOCK));
                    break;
                case 1: // XBID 8 3D Structure data: This block is only present if nExtraBlocks != 2
                    if(nExtraBlocks == 4){
                        track->nColStructures = (extraBlockOffsets[2] - extraBlockOffsets[1])/sizeof(GEOM_BLOCK);
                        track->colStructures = static_cast<GEOM_BLOCK *>(calloc(track->nColStructures, sizeof(GEOM_BLOCK)));
                        for(int structure_Idx = 0; structure_Idx < track->nColStructures; ++structure_Idx){
                            streamoff padCheck = col.tellg();
                            col.read((char*) &track->colStructures[structure_Idx].recSize, sizeof(uint32_t));
                            col.read((char*) &track->colStructures[structure_Idx].nVerts, sizeof(uint16_t));
                            col.read((char*) &track->colStructures[structure_Idx].nPoly, sizeof(uint16_t));
                            track->colStructures[structure_Idx].vertexTable = (VERT *) calloc(track->colStructures[structure_Idx].nVerts, sizeof(VERT));
                            for(int vert_Idx = 0; vert_Idx < track->colStructures[structure_Idx].nVerts; ++vert_Idx){
                                col.read((char *) &track->colStructures[structure_Idx].vertexTable[vert_Idx], sizeof(VERT));
                            }
                            track->colStructures[structure_Idx].polygonTable = static_cast<POLYGONDATA *>(calloc(track->colStructures[structure_Idx].nPoly, sizeof(POLYGONDATA)));
                            for(int poly_Idx = 0; poly_Idx < track->colStructures[structure_Idx].nPoly; ++poly_Idx){
                                col.read((char *) &track->colStructures[structure_Idx].polygonTable[poly_Idx], sizeof(POLYGONDATA));
                            }
                            col.seekg(track->colStructures[structure_Idx].recSize - (col.tellg() - padCheck), ios_base::cur); // Eat possible padding
                        }
                    }
                    break;
                case 2: // XBID 7 3D Structure Reference: This block is only present if nExtraBlocks != 2
                    if(nExtraBlocks == 4){
                        track->nColStructureReferences = (extraBlockOffsets[3] - extraBlockOffsets[2])/sizeof(GEOM_REF_BLOCK);
                        track->colStructureRefData = static_cast<GEOM_REF_BLOCK *>(calloc(track->nColStructureReferences, sizeof(GEOM_REF_BLOCK)));
                        for(int structureRef_Idx = 0; structureRef_Idx < track->nColStructures; ++structureRef_Idx){
                            streamoff padCheck = col.tellg();
                            col.read((char*) &track->colStructureRefData[structureRef_Idx].recSize, sizeof(uint16_t));
                            col.read((char*) &track->colStructureRefData[structureRef_Idx].recType, sizeof(uint8_t));
                            col.read((char*) &track->colStructureRefData[structureRef_Idx].structureRef, sizeof(uint8_t));
                            // Fixed type
                            if(track->colStructureRefData[structureRef_Idx].recType == 1){
                                col.read((char*) &track->colStructureRefData[structureRef_Idx].refCoordinates, sizeof(VERT_HIGH));
                            }
                            else if(track->colStructureRefData[structureRef_Idx].recType == 3){ // Animated type
                                col.read((char*) &track->colStructureRefData[structureRef_Idx].animLength, sizeof(uint16_t));
                                col.read((char*) &track->colStructureRefData[structureRef_Idx].unknown, sizeof(uint16_t));
                                track->colStructureRefData[structureRef_Idx].animationData = static_cast<ANIM_POS *>(calloc(track->colStructureRefData[structureRef_Idx].animLength, sizeof(ANIM_POS)));
                                for(int animation_Idx = 0; animation_Idx < track->colStructureRefData[structureRef_Idx].animLength; ++animation_Idx){
                                    col.read((char*) &track->colStructureRefData[structureRef_Idx].animationData[animation_Idx], sizeof(ANIM_POS));
                                }
                            } else {
                                std::cout << "Unknown Structure Reference type: " << (int) track->colStructureRefData[structureRef_Idx].recType << std::endl;
                            }
                            col.seekg(track->colStructureRefData[structureRef_Idx].recSize - (col.tellg() - padCheck), ios_base::cur); // Eat possible padding
                        }
                    }
                    break;
                case 3:
                    track->nCollisionData  = (nExtraBlocks == 2) ? (extraBlockOffsets[2] - extraBlockOffsets[1])/sizeof(COLLISION_BLOCK) : (extraBlockOffsets[4] - extraBlockOffsets[3])/sizeof(COLLISION_BLOCK);
                    track->collisionData = static_cast<COLLISION_BLOCK *>(calloc(track->nCollisionData, sizeof(COLLISION_BLOCK)));
                    col.read((char*) track->collisionData, track->nCollisionData * sizeof(COLLISION_BLOCK));
                    break;
                default:break;
            }
        }
        col.close();
        return true;
    }

    void dbgPrintVerts(TRACK *track, VERT_HIGH *blockReferenceCoords, const std::string &path, bool printFaces) {
        std::ofstream obj_dump;

        for(int superBlock_Idx = 0; superBlock_Idx < track->nSuperBlocks; ++superBlock_Idx){
            SUPERBLOCK superblock = track->superblocks[superBlock_Idx];
            for (int block_Idx = 0; block_Idx < superblock.nBlocks; ++block_Idx) {
                TRKBLOCK trkBlock = superblock.trackBlocks[block_Idx];
                VERT_HIGH blockReferenceCoord;
                // Print clipping rectangle
                //obj_dump << "o Block" << trkBlock.header->blockSerial << "ClippingRect" << std::endl;
                //for(int i = 0; i < 4; i++){
                //    obj_dump << "v " << trkBlock.header->clippingRect[i].x << " " << trkBlock.header->clippingRect[i].z << " " << trkBlock.header->clippingRect[i].y << std::endl;
                //}
                // obj_dump << "f " << 1+(4*trkBlock.header->blockSerial) << " " << 2+(4*trkBlock.header->blockSerial) << " " << 3+(4*trkBlock.header->blockSerial) << " " << 4+(4*trkBlock.header->blockSerial) << std::endl;
                std::ostringstream stringStream;
                stringStream << path << trkBlock.header->blockSerial << ".obj";
                obj_dump.open(stringStream.str());
                obj_dump << "o Block" << trkBlock.header->blockSerial << std::endl;
                for (int i = 0; i < trkBlock.header->nStickToNextVerts + trkBlock.header->nHighResVert; i++) {
                    if (i < trkBlock.header->nStickToNextVerts) {
                        // If in last block go get ref coord of first block, else get ref of next block
                        blockReferenceCoord =  (trkBlock.header->blockSerial == track->nBlocks-1) ? blockReferenceCoords[0] :  blockReferenceCoords[trkBlock.header->blockSerial+1];
                    } else {
                        blockReferenceCoord = blockReferenceCoords[trkBlock.header->blockSerial];
                    }
                    int32_t x = (blockReferenceCoord.x + (256 * trkBlock.vertexTable[i].x));
                    int32_t y = (blockReferenceCoord.y + (256 * trkBlock.vertexTable[i].y));
                    int32_t z = (blockReferenceCoord.z + (256 * trkBlock.vertexTable[i].z));
                    obj_dump << "v " << x << " " << z << " " << y << std::endl;
                }
                if(printFaces){
                    for (int poly_Idx = (trkBlock.header->nLowResPoly + trkBlock.header->nMedResPoly); poly_Idx < (trkBlock.header->nLowResPoly + trkBlock.header->nMedResPoly + trkBlock.header->nHighResPoly); ++poly_Idx)
                    {
                        obj_dump << "f " << (unsigned int) trkBlock.polygonTable[poly_Idx].vertex[0]+1 << " " << (unsigned int)trkBlock.polygonTable[poly_Idx].vertex[1]+1 << " " << (unsigned int) trkBlock.polygonTable[poly_Idx].vertex[2]+1 << " " << (unsigned int) trkBlock.polygonTable[poly_Idx].vertex[3]+1<< std::endl;
                    }
                }
                obj_dump.close();
                for(int structure_Idx = 0; structure_Idx < trkBlock.nStructures; ++structure_Idx){
                    std::ostringstream stringStream1;
                    stringStream1 << path << &trkBlock.structures[structure_Idx] << ".obj";
                    obj_dump.open(stringStream1.str());
                    VERT_HIGH *structureReferenceCoordinates = &blockReferenceCoords[trkBlock.header->blockSerial];
                    // Find the structure reference that matches this structure, else use block default
                    for(int structRef_Idx = 0; structRef_Idx < trkBlock.nStructureReferences; ++structRef_Idx){
                        // Only check fixed type structure references
                        if(trkBlock.structureRefData[structRef_Idx].structureRef == structure_Idx){
                            if(trkBlock.structureRefData[structRef_Idx].recType == 1){
                                structureReferenceCoordinates = &trkBlock.structureRefData[structure_Idx].refCoordinates;
                            }
                            else if(trkBlock.structureRefData[structRef_Idx].recType == 4) {
                                if(trkBlock.structureRefData[structure_Idx].animLength != 0){
                                    // For now, if animated, use position 0 of animation sequence
                                    structureReferenceCoordinates = &trkBlock.structureRefData[structure_Idx].animationData[0].position;
                                }
                            }
                        }
                    }
                    obj_dump << "o Struct" << &trkBlock.structures[structure_Idx] << std::endl;
                    for(uint16_t vert_Idx = 0; vert_Idx < trkBlock.structures[structure_Idx].nVerts; ++vert_Idx){
                        int32_t x = (structureReferenceCoordinates->x + (256 * trkBlock.structures[structure_Idx].vertexTable[vert_Idx].x));
                        int32_t y = (structureReferenceCoordinates->y + (256 *trkBlock.structures[structure_Idx].vertexTable[vert_Idx].y));
                        int32_t z = (structureReferenceCoordinates->z + (256 *trkBlock.structures[structure_Idx].vertexTable[vert_Idx].z));
                        obj_dump << "v " << x << " " << z << " " << y << std::endl;
                    }
                    if(printFaces){
                        for(int poly_Idx = 0; poly_Idx < trkBlock.structures[structure_Idx].nPoly; ++poly_Idx){
                            obj_dump << "f " << (unsigned int) trkBlock.structures[structure_Idx].polygonTable[poly_Idx].vertex[0]+1 << " " << (unsigned int)trkBlock.structures[structure_Idx].polygonTable[poly_Idx].vertex[1]+1 << " " << (unsigned int) trkBlock.structures[structure_Idx].polygonTable[poly_Idx].vertex[2]+1 << " " << (unsigned int) trkBlock.structures[structure_Idx].polygonTable[poly_Idx].vertex[3]+1 << std::endl;
                        }
                    }
                    obj_dump.close();
                }
            }
        }
        obj_dump.close();
    }
}
