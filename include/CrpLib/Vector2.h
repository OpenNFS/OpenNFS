#pragma once

#include "Interfaces.h"
#include "Common.h"
#include "Entry.h"

namespace CrpLib {

    class CVector2 :
            public ICrpData {
    private:
        tVector2 *m_pData;
        int m_Count;
        bool m_Init;

        // clean up helper
        void FreeData();

    public:

        CVector2(void);

        CVector2(tVector2 *pData, int count);

        ~CVector2(void);

        void Read(std::fstream *file, ICrpEntry *entry);

        void Write(std::fstream *file);

        int GetEntryLength();

        int GetEntryCount();

        // accessors and modifiers

        void SetItem(int index, tVector2 *value);

        tVector2 *GetItem(int index);

        int GetCount();

        void SetCount(int count);

    };

}
