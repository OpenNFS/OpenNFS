#pragma once

#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <set>
#include <stdio.h>
#include <filesystem>

#include "../Util/Utils.h"

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

class MusicLoader {
public:
    explicit MusicLoader(const std::string &song_base_path);
    void ParseMAP(const std::string &map_path, const std::string &mus_path);

private:
    uint32_t EATable[20] = {0x00000000, 0x000000F0, 0x000001CC, 0x00000188, 0x00000000, 0x00000000, 0xFFFFFF30, 0xFFFFFF24, 0x00000000, 0x00000001,
                            0x00000003, 0x00000004, 0x00000007, 0x00000008, 0x0000000A, 0x0000000B, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFD, 0xFFFFFFFC};

    uint32_t ReadBytes(FILE *file, uint8_t count);

    bool ReadSCHl(FILE *mus_file, uint32_t sch1Offset, FILE *pcm_file);

    void DecompressEAADPCM(ASFChunkHeader *asfChunkHeader, long nSamples, FILE *mus_file, FILE *pcm_file);

    void ParsePTHeader(FILE *file,
                       uint32_t *dwSampleRate,
                       uint32_t *dwChannels,
                       uint32_t *dwCompression,
                       uint32_t *dwNumSamples,
                       uint32_t *dwDataStart,
                       uint32_t *dwLoopOffset,
                       uint32_t *dwLoopLength,
                       uint32_t *dwBytesPerSample,
                       uint32_t *bSplit,
                       uint32_t *bSplitCompression);
};
