//
// Created by Amrik on 02/06/2018.
//
// Valery V. Anisimovsky (son_of_the_north@mail.ru)
// http://bim.km.ru/gap/
// http://www.anxsoft.newmail.ru
// http://anx.da.ru
// Dmitry Kirnocenskij (ejt@mail.ru)
// Worked out EA ADPCM decompression algorithm.
//
// Toni Wilen (nhlinfo@nhl-online.com)
// http://www.nhl-online.com/nhlinfo/
// Provided Valery with info on new SCDl structure, new BNK version header, PATl
// and TMpl headers. Toni Wilen is the author of SNDVIEW utility (available
// on his pages) which decompresses Electronic Arts audio files and compresses
// WAVs back into EA formats.
//
// Jesper Juul-Mortensen (jjm@danbbs.dk, ICQ#43452941)
// http://www.danbbs.dk/~jjm
// http://nfstoolbox.homepage.dk
// http://nfscheats.com/nfstoolbox
// Additional info on PT header block types. The author of utilities for NFS'x.

#include <stdio.h>
#include <cmath>
#include <math.h>
#include "music_loader.h"

#define SWAPDWORD(x) ((((x)&0xFF)<<24)+(((x)>>24)&0xFF)+(((x)>>8)&0xFF00)+(((x)<<8)&0xFF0000))
#define HINIBBLE(byte) ((byte) >> 4)
#define LONIBBLE(byte) ((byte) & 0x0F)

MusicLoader::MusicLoader(const std::string &song_base_path) {
    boost::filesystem::path p(song_base_path);
    std::string song_name = p.filename().string();
    stringstream mus_path, map_path;

    std::cout << "--- Loading Song " << song_name << "---" << std::endl;

    mus_path << song_base_path << ".mus";
    map_path << song_base_path << ".map";

    ParseMAP(map_path.str(), mus_path.str());
}

uint32_t MusicLoader::ReadBytes(FILE *file, uint8_t count) {
    uint8_t i, byte;
    uint32_t result;

    result = 0L;
    for (i = 0; i < count; i++) {
        fread(&byte, sizeof(uint8_t), 1, file);
        result <<= 8;
        result += byte;
    }

    return result;
}

// This function assumes that the current file pointer is set to the
// start of PT header data, that is, just after PT string ID "PT\0\0"
void MusicLoader::ParsePTHeader(FILE *file) {
    uint8_t byte;
    bool bInHeader, bInSubHeader;

    bInHeader = true;
    while (bInHeader) {
        fread(&byte, sizeof(uint8_t), 1, file);
        switch (byte) // parse header code
        {
            case 0xFF: // end of header
                bInHeader = false;
            case 0xFE: // skip
            case 0xFC: // skip
                break;
            case 0xFD: // subheader starts...
                bInSubHeader = true;
                while (bInSubHeader) {
                    fread(&byte, sizeof(uint8_t), 1, file);
                    switch (byte) // parse subheader code
                    {
                        case 0x82:
                            fread(&byte, sizeof(uint8_t), 1, file);
                            dwChannels = ReadBytes(file, byte);
                            break;
                        case 0x83:
                            fread(&byte, sizeof(uint8_t), 1, file);
                            dwCompression = ReadBytes(file, byte);
                            break;
                        case 0x84:
                            fread(&byte, sizeof(uint8_t), 1, file);
                            dwSampleRate = ReadBytes(file, byte);
                            break;
                        case 0x85:
                            fread(&byte, sizeof(uint8_t), 1, file);
                            dwNumSamples = ReadBytes(file, byte);
                            break;
                        case 0x86:
                            fread(&byte, sizeof(uint8_t), 1, file);
                            dwLoopOffset = ReadBytes(file, byte);
                            break;
                        case 0x87:
                            fread(&byte, sizeof(uint8_t), 1, file);
                            dwLoopLength = ReadBytes(file, byte);
                            break;
                        case 0x88:
                            fread(&byte, sizeof(uint8_t), 1, file);
                            dwDataStart = ReadBytes(file, byte);
                            break;
                        case 0x92:
                            fread(&byte, sizeof(uint8_t), 1, file);
                            dwBytesPerSample = ReadBytes(file, byte);
                            break;
                        case 0x80: // ???
                            fread(&byte, sizeof(uint8_t), 1, file);
                            bSplit = ReadBytes(file, byte);
                            break;
                        case 0xA0: // ???
                            fread(&byte, sizeof(uint8_t), 1, file);
                            bSplitCompression = ReadBytes(file, byte);
                            break;
                        case 0xFF:
                            bInSubHeader = false;
                            bInHeader = false;
                            break;
                        case 0x8A: // end of subheader
                            bInSubHeader = true;
                        default: // ???
                            fread(&byte, sizeof(uint8_t), 1, file);
                            fseek(file, byte, SEEK_CUR);
                    }
                }
                break;
            default:
                fread(&byte, sizeof(uint8_t), 1, file);
                if (byte == 0xFF)
                    fseek(file, 4, SEEK_CUR);
                fseek(file, byte, SEEK_CUR);
        }
    }
}

void write_little_endian(unsigned int word, int num_bytes, FILE *wav_file) {
    unsigned buf;
    while (num_bytes > 0) {
        buf = word & 0xff;
        fwrite(&buf, 1, 1, wav_file);
        num_bytes--;
        word >>= 8;
    }
}

void write_shit(unsigned int word, FILE*file){
    fwrite(&word, 2, 1, file);
}

/* make_wav.c
 * Creates a WAV file from an array of ints.
 * Output is monophonic, signed 16-bit samples
 * copyright
 * Fri Jun 18 16:36:23 PDT 2010 Kevin Karplus
 * Creative Commons license Attribution-NonCommercial
 *  http://creativecommons.org/licenses/by-nc/3.0/
 */
void start_write_wav(FILE *wav_file, unsigned long num_samples, int s_rate) {
    unsigned int sample_rate;
    unsigned int num_channels;
    unsigned int bytes_per_sample;
    unsigned int byte_rate;
    unsigned long i;    /* counter for samples */

    num_channels = 1;   /* monoaural */
    bytes_per_sample = 2;

    if (s_rate <= 0) sample_rate = 44100;
    else sample_rate = (unsigned int) s_rate;

    byte_rate = sample_rate * num_channels * bytes_per_sample;

    assert(wav_file);   /* make sure it opened */

    /* write RIFF header */
    fwrite("RIFF", 1, 4, wav_file);
    write_little_endian(36 + bytes_per_sample * num_samples * num_channels, 4, wav_file);
    fwrite("WAVE", 1, 4, wav_file);

    /* write fmt  subchunk */
    fwrite("fmt ", 1, 4, wav_file);
    write_little_endian(16, 4, wav_file);   /* SubChunk1Size is 16 */
    write_little_endian(1, 2, wav_file);    /* PCM is format 1 */
    write_little_endian(num_channels, 2, wav_file);
    write_little_endian(sample_rate, 4, wav_file);
    write_little_endian(byte_rate, 4, wav_file);
    write_little_endian(num_channels * bytes_per_sample, 2, wav_file);  /* block align */
    write_little_endian(8 * bytes_per_sample, 2, wav_file);  /* bits/sample */

    /* write data subchunk */
    fwrite("data", 1, 4, wav_file);
    write_little_endian(bytes_per_sample * num_samples * num_channels, 4, wav_file);
}

int32_t Clip16BitSample(int32_t sample) {
    if (sample > 32767)
        return 32767;
    else if (sample < -32768)
        return (-32768);
    else
        return sample;
}

void MusicLoader::DecompressEAADPCM(ASFChunkHeader *asfChunkHeader, long nSamples, FILE* mus_file) {
    // 22050 Hz
    //stringstream wav_path;
    //wav_path << asfChunkHeader->dwOutSize * asfChunkHeader->lCurSampleLeft << ".wav";
    //FILE *wav_file = fopen(wav_path.str().c_str(), "w");
    //start_write_wav(wav_file, nSamples, 22050);

    uint32_t l = 0, r =0;
    uint16_t *outBufL = (uint16_t*) calloc(nSamples, sizeof(uint16_t));
    uint16_t *outBufR = (uint16_t*) calloc(nSamples, sizeof(uint16_t));


    // TODO: Different stuff for MONO/Stereo
    int32_t lCurSampleLeft = asfChunkHeader->lCurSampleLeft;
    int32_t lCurSampleRight = asfChunkHeader->lCurSampleRight;
    int32_t lPrevSampleLeft = asfChunkHeader->lPrevSampleLeft;
    int32_t lPrevSampleRight = asfChunkHeader->lPrevSampleRight;

    uint8_t bInput;
    int32_t c1left, c2left, c1right, c2right, left, right;
    uint8_t dleft, dright;
    uint32_t dwSubOutSize = 0x1c;

    // process integral number of (dwSubOutSize) samples
    for (uint32_t bCount = 0; bCount < (asfChunkHeader->dwOutSize / dwSubOutSize); bCount++) {
        fread(&bInput, sizeof(int8_t), 1, mus_file);
        c1left = EATable[HINIBBLE(bInput)];   // predictor coeffs for left channel
        c2left = EATable[HINIBBLE(bInput) + 4];
        c1right = EATable[LONIBBLE(bInput)];  // predictor coeffs for right channel
        c2right = EATable[LONIBBLE(bInput) + 4];
        fread(&bInput, sizeof(int8_t), 1, mus_file);
        dleft = HINIBBLE(bInput) + 8;   // shift value for left channel
        dright = LONIBBLE(bInput) + 8;  // shift value for right channel
        for (uint32_t sCount = 0; sCount < dwSubOutSize; sCount++) {
            fread(&bInput, sizeof(int8_t), 1, mus_file);
            left = HINIBBLE(bInput);  // HIGHER nibble for left channel
            right = LONIBBLE(bInput); // LOWER nibble for right channel
            left = (left << 0x1c) >> dleft;
            right = (right << 0x1c) >> dright;
            left = (left + lCurSampleLeft * c1left + lPrevSampleLeft * c2left + 0x80) >> 8;
            right = (right + lCurSampleRight * c1right + lPrevSampleRight * c2right + 0x80) >> 8;
            left = Clip16BitSample(left);
            right = Clip16BitSample(right);
            lPrevSampleLeft = lCurSampleLeft;
            lCurSampleLeft = left;
            lPrevSampleRight = lCurSampleRight;
            lCurSampleRight = right;

            // Now we've got lCurSampleLeft and lCurSampleRight which form one stereo
            // sample and all is set for the next input byte...
            std::cout << lCurSampleLeft << " " << lCurSampleRight << std::endl; // send the sample to output
            //write_little_endian((uint16_t) lCurSampleLeft, 2, wav_file);
            outBufL[l++] = (uint16_t)lCurSampleLeft;
            outBufR[r++] = (uint16_t)lCurSampleRight;
        }
    }

    // process the rest (if any)
    if ((asfChunkHeader->dwOutSize % dwSubOutSize) != 0) {
        fread(&bInput, sizeof(int8_t), 1, mus_file);
        //bInput=audioData[i++];
        c1left = EATable[HINIBBLE(bInput)];   // predictor coeffs for left channel
        c2left = EATable[HINIBBLE(bInput) + 4];
        c1right = EATable[LONIBBLE(bInput)];  // predictor coeffs for right channel
        c2right = EATable[LONIBBLE(bInput) + 4];
        fread(&bInput, sizeof(int8_t), 1, mus_file);
        //bInput=audioData[i++];
        dleft = HINIBBLE(bInput) + 8;   // shift value for left channel
        dright = LONIBBLE(bInput) + 8;  // shift value for right channel
        for (uint32_t sCount = 0; sCount < (asfChunkHeader->dwOutSize % dwSubOutSize); sCount++) {
            fread(&bInput, sizeof(int8_t), 1, mus_file);
            left = HINIBBLE(bInput);  // HIGHER nibble for left channel
            right = LONIBBLE(bInput); // LOWER nibble for right channel
            left = (left << 0x1c) >> dleft;
            right = (right << 0x1c) >> dright;
            left = (left + lCurSampleLeft * c1left + lPrevSampleLeft * c2left + 0x80) >> 8;
            right = (right + lCurSampleRight * c1right + lPrevSampleRight * c2right + 0x80) >> 8;
            left = Clip16BitSample(left);
            right = Clip16BitSample(right);
            lPrevSampleLeft = lCurSampleLeft;
            lCurSampleLeft = left;
            lPrevSampleRight = lCurSampleRight;
            lCurSampleRight = right;

            // Now we've got lCurSampleLeft and lCurSampleRight which form one stereo
            // sample and all is set for the next input byte...
            std::cout << lCurSampleLeft << " " << lCurSampleRight << std::endl; // send the sample to output
            //write_little_endian((uint16_t) lCurSampleLeft, 2, wav_file);
            outBufL[l++] = (uint16_t)lCurSampleLeft;
            outBufR[r++] = (uint16_t)lCurSampleRight;
        }
    }

    // TODO: PLay OUTBUFL and OUTBUFR using OpenAL
    stringstream wav_path;
    wav_path << asfChunkHeader->dwOutSize * asfChunkHeader->lCurSampleLeft << ".pcm";
    FILE *wav_file = fopen(wav_path.str().c_str(), "w");
    for (int t = 0; t < nSamples; ++t)
    {
        short s16 = outBufL[t];
        unsigned char c;
        c = (unsigned)s16 % 256;
        fwrite(&c, 1, 1, wav_file);
        c = (unsigned)s16 / 256 % 256;
        fwrite(&c, 1, 1, wav_file);
    }
    fclose(wav_file);

    free(outBufL);
    free(outBufR);
}

bool MusicLoader::ReadSCHl(FILE *mus_file, uint32_t sch1Offset) {
    fseek(mus_file, static_cast<long>(sch1Offset), SEEK_SET);

    ASFBlockHeader *chk = (ASFBlockHeader *) calloc(1, sizeof(ASFBlockHeader));
    fread(chk, sizeof(ASFBlockHeader), 1, mus_file);
    if (memcmp(chk->szBlockID, "SCHl", sizeof(chk->szBlockID)) != 0) {
        free(chk);
        fclose(mus_file);
        return false;
    }

    char blockIDString[4];
    // Check ID string is PT
    fread(blockIDString, sizeof(char), 4, mus_file);
    ParsePTHeader(mus_file);

    uint32_t sch1Size = chk->dwSize;
    // Jump to next block
    fseek(mus_file, static_cast<long>(sch1Size), SEEK_SET);

    // Check in SCC1 Count block
    fread(chk, sizeof(ASFBlockHeader), 1, mus_file);
    if (memcmp(chk->szBlockID, "SCCl", sizeof(chk->szBlockID)) != 0) {
        free(chk);
        fclose(mus_file);
        return false;
    }
    uint32_t scc1Size = chk->dwSize;
    uint8_t nSCD1Blocks;
    fread(&nSCD1Blocks, sizeof(uint8_t), 1, mus_file);

    long totalSCD1InterleaveSize = 0;

    // TODO: Add a check for compression flag = 0x7, EADPCM decode
    // Get PCM data from SCD1 blocks
    for (uint8_t scd1_Idx = 0; scd1_Idx < nSCD1Blocks; ++scd1_Idx) {
        // Jump to next block
        fseek(mus_file, static_cast<long>(sch1Size + scc1Size + totalSCD1InterleaveSize), SEEK_SET);

        // Check in SCD1
        fread(chk, sizeof(ASFBlockHeader), 1, mus_file);
        if (memcmp(chk->szBlockID, "SCDl", sizeof(chk->szBlockID)) != 0) {
            free(chk);
            fclose(mus_file);
            return false;
        }

        ASFChunkHeader *asfChunkHeader = (ASFChunkHeader *) calloc(1, sizeof(ASFChunkHeader));
        fread(asfChunkHeader, sizeof(ASFChunkHeader), 1, mus_file);
        DecompressEAADPCM(asfChunkHeader, chk->dwSize - sizeof(ASFBlockHeader)-sizeof(ASFChunkHeader), mus_file);
        totalSCD1InterleaveSize += chk->dwSize;

        free(asfChunkHeader);
    }
    fseek(mus_file, static_cast<long>(sch1Size + scc1Size + totalSCD1InterleaveSize), SEEK_SET);

    // Check we successfully reached end block SCEl
    fread(chk, sizeof(ASFBlockHeader), 1, mus_file);
    bool success = memcmp(chk->szBlockID, "SCEl", sizeof(chk->szBlockID)) == 0;
    free(chk);
    return success;
}

void MusicLoader::ParseMAP(const std::string &map_path, const std::string &mus_path) {
    std::cout << "- Parsing MAP File " << std::endl;
    ifstream map(map_path, ios::in | ios::binary);

    // Read the MAP file header
    MAPHeader *mapHeader = static_cast<MAPHeader *>(calloc(1, sizeof(MAPHeader)));
    map.read((char *) mapHeader, sizeof(MAPHeader));
    std::cout << (int) mapHeader->bNumSections << " Sections" << std::endl;

    MAPSectionDef *sectionDefTable = static_cast<MAPSectionDef *>(calloc(mapHeader->bNumSections,
                                                                         sizeof(MAPSectionDef)));
    map.read((char *) sectionDefTable, mapHeader->bNumSections * sizeof(MAPSectionDef));

    // Skip over seemlingly useless records
    map.seekg(mapHeader->bNumRecords * 0x10, ios_base::cur); // bRecordSize may be incorrect, use 0x10 to be safe

    uint32_t *startingPositions = static_cast<uint32_t *>(calloc(mapHeader->bNumSections, sizeof(long)));

    for (int startPos_Idx = 0; startPos_Idx < mapHeader->bNumSections; ++startPos_Idx) {
        uint32_t startingPosition;
        map.read((char *) &startingPosition, sizeof(uint32_t));
        startingPositions[startPos_Idx] = SWAPDWORD(startingPosition);
    }

    std::cout << "MAP File successfully parsed. Playing back MUS file." << std::endl;

    FILE *mus_file = fopen(mus_path.c_str(), "rb");

    // Get starting position of first section
    uint8_t section_Idx = mapHeader->bFirstSection;

    while (sectionDefTable[section_Idx].bNumRecords > 0) {
        // Starting positions are raw offsets into MUS file
        // Read the SCH1 header and further blocks in MUS to play the section
        if (!ReadSCHl(mus_file, startingPositions[section_Idx])) { //
            std::cout << "Error reading SCHl block." << std::endl;
            break;
        }
        section_Idx = sectionDefTable[section_Idx].msdRecords[sectionDefTable[section_Idx].bNumRecords -
                                                              1].bNextSection;
        // TODO: We should loop if we come across a section we've already played. Add to set and check presence? (or break)
    }

    free(startingPositions);
    free(sectionDefTable);
    fclose(mus_file);
}
