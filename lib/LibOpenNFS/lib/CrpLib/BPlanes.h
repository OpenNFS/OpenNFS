#pragma once

#include "Interfaces.h"
#include "Common.h"
#include "Entry.h"

#include <fstream>

namespace CrpLib {
    class CBPlanes : public ICrpData {
    private:
        int m_Unk1, m_Unk2;
        int m_Count;
        tVector4 *m_pVertices;
        bool m_Init;
        // clean up helper
        void FreeData();
    public:
        CBPlanes(void);
        ~CBPlanes(void);
        void Read(std::fstream *file, ICrpEntry *entry);
        void Write(std::fstream *file);
        int GetEntryLength();
        int GetEntryCount();
        void SetCount(int value);
        int GetCount();
        tVector4 *GetVertices();
    };
}