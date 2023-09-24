#pragma once

#include "Interfaces.h"
#include "Common.h"
#include "Entry.h"

namespace CrpLib {

    class CVector4 :
            public ICrpData {
    private:
        tVector4 *m_pData;
        int m_Count;
        bool m_Init;

        // clean up helper
        void FreeData();

    public:

        CVector4(void);

        CVector4(tVector4 *pData, int count);

        ~CVector4(void);

        void Read(std::fstream *file, ICrpEntry *entry);

        void Write(std::fstream *file);

        int GetEntryLength();

        int GetEntryCount();

        // accessors and modifiers

        void SetItem(int index, tVector4 *value);

        tVector4 *GetItem(int index);

        int GetCount();

        void SetCount(int count);

    };

}
