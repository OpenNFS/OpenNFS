//
// Created by Amrik on 24/05/2018.
//

// TODO: Base class the track loader and extend for each NFS, so dont have to carry this shitty class name around
#include <iostream>
#include "nfs2_trk_loader.h"

using namespace std;

nfs2_trk_loader::nfs2_trk_loader(const std::string &trk_path) {
    std::cout << "Loading NFS2 Track.. " << std::endl;
    LoadTRK(trk_path);
}

bool nfs2_trk_loader::LoadTRK(std::string trk_path) {
    ifstream ar(trk_path, ios::in | ios::binary);
    // Check we're in a valid TRK file
    unsigned char header[4];
    if (ar.read(((char *) header), sizeof(unsigned char) * 4).gcount() != sizeof(unsigned char) * 4) return false;
    if (!strcmp(reinterpret_cast<const char *>(header), "TRAC")) return false;

    // Unknown header data
    if (ar.read(((char *) unknownHeader), sizeof(uint32_t) * 5).gcount() != sizeof(uint32_t) * 5) return false;

    // Basic Track data
    ar.read((char *) &nSuperBlocks, sizeof(uint32_t));
    ar.read((char *) &nBlocks, sizeof(uint32_t));
    superblocks = (NFS2_SUPERBLOCK *) calloc(static_cast<size_t>(nBlocks), sizeof(NFS2_SUPERBLOCK));

    // Offsets of Superblocks in TRK file
    uint32_t superblockOffsets[nSuperBlocks];
    if (ar.read(((char *) superblockOffsets), nSuperBlocks * sizeof(uint32_t)).gcount() !=
        nSuperBlocks * sizeof(uint32_t))
        return false;

    // Reference coordinates for each block
    NFS2_VERT_HIGH *blockReferenceCoords = (NFS2_VERT_HIGH *) calloc(static_cast<size_t>(nBlocks), sizeof(NFS2_VERT_HIGH));
    if (ar.read(((char *) blockReferenceCoords), nBlocks * sizeof(NFS2_VERT_HIGH)).gcount() != nBlocks * sizeof(NFS2_VERT_HIGH)){
        free(blockReferenceCoords);
        return false;
    }

    for (int superBlock_Idx = 0; superBlock_Idx < nSuperBlocks; ++superBlock_Idx) {
        std::cout << "SuperBlock " << superBlock_Idx << " of " << nSuperBlocks << std::endl;
        // Get the superblock header
        NFS2_SUPERBLOCK *superblock = &superblocks[superBlock_Idx];
        ar.seekg(superblockOffsets[superBlock_Idx], ios_base::beg);
        ar.read((char *) &superblock->superBlockSize, sizeof(uint32_t));
        ar.read((char *) &superblock->nBlocks, sizeof(uint32_t));
        ar.read((char *) &superblock->padding, sizeof(uint32_t));

        if (superblock->nBlocks != 0) {
            // Get the offsets of the child blocks within superblock
            uint32_t *blockOffsets = (uint32_t *) calloc(static_cast<size_t>(superblock->nBlocks), sizeof(uint32_t));
            ar.read((char *) blockOffsets, superblock->nBlocks * sizeof(uint32_t));
            superblock->trackBlocks = (NFS2_TRKBLOCK *) calloc(static_cast<size_t>(superblock->nBlocks), sizeof(NFS2_TRKBLOCK));

            for (int block_Idx = 0; block_Idx < superblock->nBlocks; ++block_Idx) {
                std::cout << "  Block " << block_Idx << " of " << superblock->nBlocks << std::endl;
                NFS2_TRKBLOCK *trackblock = &superblock->trackBlocks[block_Idx];
                // Read Header
                trackblock->header = (NFS2_TRKBLOCK_HEADER *) calloc(1, sizeof(NFS2_TRKBLOCK_HEADER));
                ar.seekg((uint16_t) superblockOffsets[superBlock_Idx] + blockOffsets[block_Idx], ios_base::beg);
                ar.read((char *) trackblock->header, sizeof(NFS2_TRKBLOCK_HEADER));

                // Sanity Checks
                if((trackblock->header->blockSize != trackblock->header->blockSizeDup)||(trackblock->header->blockSerial > nBlocks)){
                    std::cout<< "   --- Bad Block" << std::endl;
                    //dbgPrintVerts(superblocks, superBlock_Idx, blockReferenceCoords);
                    continue;
                }

                // Read 3D Data
                trackblock->vertexTable = (NFS2_VERT *) calloc(static_cast<size_t>(trackblock->header->nStickToNextVerts + trackblock->header->nHighResVert), sizeof(NFS2_VERT));
                for(int vert_Idx = 0; vert_Idx < trackblock->header->nStickToNextVerts + trackblock->header->nHighResVert; ++vert_Idx){
                    ar.read((char *) &trackblock->vertexTable[vert_Idx], sizeof(NFS2_VERT));
                }

                trackblock->polygonTable = (NFS2_POLYGONDATA *) calloc(static_cast<size_t>(trackblock->header->nLowResPoly + trackblock->header->nMedResPoly + trackblock->header->nHighResPoly), sizeof(NFS2_POLYGONDATA));
                for(int poly_Idx = 0; poly_Idx < (trackblock->header->nLowResPoly + trackblock->header->nMedResPoly + trackblock->header->nHighResPoly); ++poly_Idx){
                    ar.read((char *) &trackblock->polygonTable[poly_Idx], sizeof(NFS2_POLYGONDATA));
                }

                // Extrablock data
                //ar.seekg((uint16_t) superblockOffsets[superBlock_Idx] + blockOffsets[block_Idx] + trackblock->header->extraBlockTblOffset, ios_base::beg);
            }
        }
    }
    //dbgPrintVerts(superblocks, nSuperBlocks, blockReferenceCoords);
    //std::cout << "lol";
    ar.close();
}

void dbgPrintVerts(NFS2_SUPERBLOCK *superblocks, long nSuperBlocks, NFS2_VERT_HIGH *blockReferenceCoords) {
    for(int superBlock_Idx = 0; superBlock_Idx < nSuperBlocks; ++superBlock_Idx){
        std::cout << "g SuperBlock" << superBlock_Idx << std::endl;
        NFS2_SUPERBLOCK superblock = superblocks[superBlock_Idx];
        for (int block_Idx = 0; block_Idx < superblock.nBlocks; ++block_Idx) {
            NFS2_TRKBLOCK trkBlock = superblock.trackBlocks[block_Idx];
            NFS2_VERT_HIGH blockReferenceCoord = blockReferenceCoords[trkBlock.header->blockSerial];

            std::cout << "o Block" << trkBlock.header->blockSerial << std::endl;
            for (int i = 0; i < trkBlock.header->nStickToNextVerts + trkBlock.header->nHighResVert; i++) {
                if (i < trkBlock.header->nStickToNextVerts) {
                    if (block_Idx == superblock.nBlocks - 1){
                        // If in last superblock, go get block in first super block ref coords
                        if(superBlock_Idx == nSuperBlocks - 1){
                            blockReferenceCoord = blockReferenceCoords[0];
                        } else { // Else get reference coords of first block in next superblock
                            blockReferenceCoord = blockReferenceCoords[(superBlock_Idx+1)*8];
                        }
                    }
                    std::cout << "v " <<
                              blockReferenceCoord.x +
                              (256 * trkBlock.vertexTable[i].x) << " " <<
                              blockReferenceCoord.y +
                              (256 * trkBlock.vertexTable[i].y) << " " <<
                              blockReferenceCoord.z +
                              (256 * trkBlock.vertexTable[i].z) << std::endl;
                }
            }

            for (int poly_Idx = (short) (trkBlock.header->nLowResPoly + trkBlock.header->nMedResPoly); poly_Idx < (short) (trkBlock.header->nLowResPoly + trkBlock.header->nMedResPoly + trkBlock.header->nHighResPoly); ++poly_Idx)
            {
                //std::cout << "f " << (int) trkBlock.polygonTable[poly_Idx].vertex[0] << " " << (int)trkBlock.polygonTable[poly_Idx].vertex[1] << " " << (int) trkBlock.polygonTable[poly_Idx].vertex[2] << std::endl;
                //std::cout << "f " << (int) trkBlock.polygonTable[poly_Idx].vertex[0] << " " << (int)trkBlock.polygonTable[poly_Idx].vertex[2] << " " << (int) trkBlock.polygonTable[poly_Idx].vertex[3] << std::endl;
            }
        }
    }
}
