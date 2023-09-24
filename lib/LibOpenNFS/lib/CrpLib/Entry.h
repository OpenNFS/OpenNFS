#pragma once

#include "Common.h"
#include "Interfaces.h"

#include <vector>
#include <fstream>

using namespace std;

namespace CrpLib {
    class CEntry : public ICrpEntry {
    private:
        ENTRY_ID m_Id;
        int m_Index, m_Flags, m_Length, m_Count, m_Offs;
        int m_RealOffs;

        vector<ICrpEntry *> m_SubEntries;
        ICrpData *m_pData;

        void InsertSubEntry(CEntry *newEn);

    public:
        CEntry(void);

        CEntry(ENTRY_ID id, int index);

        ~CEntry(void);

        void Read(std::fstream *file);

        void WriteEntry(std::fstream *file);

        void WriteData(std::fstream *file);

        // -- accessors --

        ENTRY_ID GetId();

        int GetIndex();

        int GetLength();

        int GetCount();

        int GetFlags();

        bool IsDataEntry();

        ICrpData *GetData();

        // -- modifiers --

        void SetLength(int length);

        void SetCount(int count);

        void SetFlags(int flags);

        void SetData(ICrpData *pData);

        void SetTargetOffs(int Offs);

        // -- worker methods --

        CEntry *GetSubEntry(int index);

        std::string GetEntryType();

        CEntry *GetPartEntry(int Level, int PartIndex);

        CEntry *GetDataEntry(ENTRY_ID Id, int Level, int AnimIndex = 0, bool Damaged = false);

        CEntry *GetSubEntry(ENTRY_ID Id, int Index = 0);

        // -- new methods --

        CEntry *NewSubEntry(ENTRY_ID Id, int Index = 0, bool Allocate = true);

        CEntry *NewDataEntry(ENTRY_ID Id, int Level, int AnimIndex = 0, bool Damaged = false, bool Anim = false,
                             bool Allocate = true);

        CEntry *NewPartEntry(int Level, int PartIndex, bool Allocate = true);

    };

}
