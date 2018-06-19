//
// Created by Amrik on 26/02/2018.
//

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#ifndef OPENNFS3_NFS_DATA_H
#define OPENNFS3_NFS_DATA_H

#include <cstdint>
#include <map>
#include <vector>
#include <GL/glew.h>

class TrackBlock;

enum NFSVer { NFS_1, NFS_2, NFS_3, NFS_4, NFS_5};

// ---- NFS2/3 GL Structures -----
class Texture {
public:
    unsigned int texture_id, width, height;
    GLubyte *texture_data;

    Texture() = default;

    explicit Texture(unsigned int id, GLubyte *data, unsigned int w, unsigned int h) {
        texture_id = id;
        texture_data = data;
        width = w;
        height = h;
    }
};


namespace Music {
    typedef struct MAPHeader {
        char szID[4];
        uint8_t bUnknown1;
        uint8_t bFirstSection;
        uint8_t bNumSections;
        uint8_t bRecordSize; // ???
        uint8_t Unknown2[3];
        uint8_t bNumRecords;
    } MAPHeader;

    typedef struct MAPSectionDefRecord {
        uint8_t bUnknown;
        uint8_t bMagic;
        uint8_t bNextSection;
    } MAPSectionDefRecord;

    typedef struct MAPSectionDef {
        uint8_t bIndex;
        uint8_t bNumRecords;
        uint8_t szID[2];
        struct MAPSectionDefRecord msdRecords[8];
    } MAPSectionDef;

    struct ASFBlockHeader {
        char szBlockID[4];
        uint32_t dwSize;
    };

    struct ASFChunkHeader {
        uint32_t dwOutSize;
        uint16_t lCurSampleLeft;
        uint16_t lPrevSampleLeft;
        uint16_t lCurSampleRight;
        uint16_t lPrevSampleRight;
    };


}

#endif //OPENNFS3_NFS_DATA_H

#pragma clang diagnostic pop