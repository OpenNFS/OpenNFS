#pragma once

#include "Interfaces.h"
#include "Common.h"

#include <memory.h>
#include <fstream>

namespace CrpLib {

    class CBase :
            public ICrpData {
    private:

        int m_Flags;    // BASE_FLAGS
        int m_Zero1, m_Zero2;
        int m_LevelCount;

        int m_HasFloats;
        float m_pFloats[12];

        tBaseInfo m_BaseInfo;

        tVector4 m_Unk;

        tLevelMask *m_pLevMasks;

        bool m_Init;

        // clean up helper
        void FreeData();

    public:
        /// functions start here ///
        CBase(void);

        ~CBase(void);

        void Read(std::fstream *file, ICrpEntry *entry);

        void Write(std::fstream *file);

        int GetEntryLength();

        int GetEntryCount();

        void CreateLevels(int LevCount, int Start = 1);

        int GetLevelCount();

        tLevelMask *GetLevel(int i);

        BASE_FLAGS GetFlags();

        void SetFlags(BASE_FLAGS flags);

        void SetHasFloats(bool val);

        float *GetFloats();

        tBaseInfo *GetBaseInfo();
    };
}
