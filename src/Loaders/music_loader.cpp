//
// Created by Amrik on 02/06/2018.
//

#include "music_loader.h"

#define SWAPDWORD(x) ((((x)&0xFF)<<24)+(((x)>>24)&0xFF)+(((x)>>8)&0xFF00)+(((x)<<8)&0xFF0000))

namespace Music {
    void ParseMAP(const std::string &map_path) {
        std::cout << "- Parsing MAP File " << std::endl;
        ifstream map(map_path, ios::in | ios::binary);

        // Read the MAP file header
        MAPHeader *mapHeader = static_cast<MAPHeader *>(calloc(1, sizeof(MAPHeader)));
        map.read((char *) mapHeader, sizeof(MAPHeader));

        MAPSectionDef *sectionDefTable = static_cast<MAPSectionDef *>(calloc(mapHeader->bNumSections, sizeof(MAPSectionDef)));
        map.read((char*) sectionDefTable, mapHeader->bNumSections * sizeof(MAPSectionDef));

        // Skip over seemlingly useless records
        map.seekg(mapHeader->bNumRecords * mapHeader->bRecordSize, ios_base::cur); // bRecordSize may be incorrect, use 0x10 to be safe

        streamoff pos = map.tellg();
        map.seekg(ios_base::end);

        uint32_t nStartingPositions = static_cast<uint32_t>((map.tellg() - pos) / sizeof(uint32_t));
        map.seekg(pos, ios_base::beg);
        uint32_t *startingPositions = static_cast<uint32_t *>(calloc(nStartingPositions, sizeof(uint32_t)));

        for(int startPos_Idx = 0; startPos_Idx < nStartingPositions; ++startPos_Idx){
            uint32_t startingPosition;
            map.read((char*) &startingPosition, sizeof(uint32_t));
            startingPositions[startPos_Idx] = SWAPDWORD(startingPosition);
        }

        std::cout << "Done" << std::endl;
        free(startingPositions);
    }
}
