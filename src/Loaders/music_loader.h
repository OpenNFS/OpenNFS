//
// Created by Amrik on 02/06/2018.
//

#ifndef OPENNFS3_MAP_LOADER_H
#define OPENNFS3_MAP_LOADER_H

#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <boost/filesystem/path.hpp>
#include <sstream>
#include "../nfs_data.h"

using namespace std;
using namespace Music;

class MusicLoader{
public:
    explicit MusicLoader(const std::string &song_base_path);
    void ParseMAP(const std::string &map_path, const std::string &mus_path);

private:
    uint32_t  dwSampleRate = 0;
    uint32_t  dwChannels = 0;
    uint32_t  dwCompression = 0;
    uint32_t  dwNumSamples = 0;
    uint32_t  dwDataStart = 0;
    uint32_t  dwLoopOffset = 0;
    uint32_t  dwLoopLength = 0;
    uint32_t  dwBytesPerSample = 0;
    uint32_t bSplit = 0;
    uint32_t bSplitCompression = 0;

    uint32_t ReadBytes(FILE* file, uint8_t count);

    bool ReadSCHl(FILE *mus_file, uint32_t sch1Offset);

    void ParsePTHeader(FILE* file);
};

#endif //OPENNFS3_MAP_LOADER_H
