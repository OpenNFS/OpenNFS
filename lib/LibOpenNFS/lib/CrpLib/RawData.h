#pragma once

#include "Interfaces.h"
#include "Entry.h"

#include <string>
#include <fstream>

namespace CrpLib {

    class CRawData : public ICrpData {
    private:
        static const uint32_t RAWDATA_MAGIC = 0x57415243;
        char *m_pData;
        std::streamoff m_Length;
        bool m_Init;
        // clean up helper
        void FreeData();

    public:
        CRawData(void);
        CRawData(char *pData, int length);
        ~CRawData(void);
        void Read(std::fstream *file, ICrpEntry *entry);
        void Write(std::fstream *file);
        int GetEntryLength();
        int GetEntryCount();
        bool ReadFromFile(std::string filename);
        bool WriteToFile(std::string filename);
        void ParseFrom(ICrpEntry *entry);
        void ParseTo(ICrpEntry *entry);

        // accessors and modifiers
        void SetData(char *pData, int length);
        void SetData(std::string data);
        char *GetData();
        int GetLength();
    };

}
