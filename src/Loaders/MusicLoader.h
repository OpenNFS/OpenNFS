//
// Created by Amrik Sadhra on 02/06/2018.
//

#pragma once

#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <boost/filesystem/path.hpp>
#include <sstream>
#include <stdio.h>
#include "../nfs_data.h"
#include "../Util/Utils.h"
#include <set>

using namespace std;
using namespace Music;

class MusicLoader{
public:
    explicit MusicLoader(const std::string &song_base_path);
    void ParseMAP(const std::string &map_path, const std::string &mus_path);

private:
    uint32_t EATable[20] =
            {
                    0x00000000,
                    0x000000F0,
                    0x000001CC,
                    0x00000188,
                    0x00000000,
                    0x00000000,
                    0xFFFFFF30,
                    0xFFFFFF24,
                    0x00000000,
                    0x00000001,
                    0x00000003,
                    0x00000004,
                    0x00000007,
                    0x00000008,
                    0x0000000A,
                    0x0000000B,
                    0x00000000,
                    0xFFFFFFFF,
                    0xFFFFFFFD,
                    0xFFFFFFFC
            };
   
    uint32_t ReadBytes(FILE* file, uint8_t count);

    bool ReadSCHl(FILE *mus_file, uint32_t sch1Offset, FILE* pcm_file);

    void DecompressEAADPCM(ASFChunkHeader *asfChunkHeader, long nSamples, FILE* mus_file, FILE *pcm_file);

    void ParsePTHeader(FILE* file, uint32_t  *dwSampleRate, uint32_t  *dwChannels, uint32_t  *dwCompression, uint32_t  *dwNumSamples, uint32_t  *dwDataStart, uint32_t  *dwLoopOffset, uint32_t  *dwLoopLength, uint32_t *dwBytesPerSample, uint32_t  *bSplit, uint32_t  *bSplitCompression);
};
