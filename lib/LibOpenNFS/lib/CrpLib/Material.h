#pragma once

#include "Interfaces.h"

#include <fstream>

namespace CrpLib {

    class CMaterial :
            public ICrpData {
    private:
        char m_pData1[0x10];
        char m_pRMthName[0x10];
        char m_pData2[0x8];
        int m_TpgIndex;
        char m_pData3[0x10C];

    public:
        CMaterial(void);

        ~CMaterial(void);

        void Read(std::fstream *file, ICrpEntry *entry);

        void Write(std::fstream *file);

        int GetEntryLength();

        int GetEntryCount();

        int GetTpgIndex();

        void SetTpgIndex(int index);

        void SetCull(bool cull);

        char *GetRenderMethodName();

    };

}