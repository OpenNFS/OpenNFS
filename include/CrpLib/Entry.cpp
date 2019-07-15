#include "Entry.h"

namespace CrpLib {

    CEntry::CEntry(void) {
        m_Id = (ENTRY_ID) 0;
        m_Index = 0;
        m_Length = 0;
        m_Count = 0;

        m_pData = NULL;
    }

    CEntry::CEntry(ENTRY_ID id, int index) {
        m_Id = id;
        m_Index = index;
        m_Length = 0;
        m_Count = 0;

        m_pData = NULL;
    }

    CEntry::~CEntry(void) {
        if (m_Length == 0) {
            for (int i = 0; i < m_Count; i++) {
                delete ((ICrpEntry *) m_SubEntries[i]);
            }
            m_SubEntries.clear();
        } else {
            delete m_pData;
        }
    }

    void CEntry::Read(std::fstream *file) {

        int tmp1, tmp2;

        m_RealOffs = (int) file->tellg();

        file->read((char *) &tmp1, 4);
        file->read((char *) &tmp2, 4);

        m_Flags = tmp2 & 0xFF;
        m_Length = tmp2 >> 8;

        if (m_Flags & 0x1) {
            // id + index
            m_Index = tmp1 & 0xFFFF;
            m_Id = (ENTRY_ID) (tmp1 >> 16);
        } else {
            // just id
            m_Index = 0;
            m_Id = (ENTRY_ID) tmp1;
        }

        file->read((char *) &m_Count, 4);
        file->read((char *) &m_Offs, 4);

        if (m_Length == 0) {
            // has subentries

            m_Offs <<= 4;
            m_pData = NULL;

            file->seekg(m_RealOffs + m_Offs, std::ios::beg);

            m_SubEntries.reserve(m_Count);

            ICrpEntry *en;
            for (int i = 0; i < m_Count; i++) {
                en = new CEntry();
                en->Read(file);
                m_SubEntries.push_back(en);
            }

            file->seekg(m_RealOffs + 0x10, std::ios::beg);

        } else {
            // has data

            file->seekg(m_RealOffs + m_Offs, std::ios::beg);

            m_pData = AllocateDataEntry(m_Id);
            m_pData->Read(file, this);

            file->seekg(m_RealOffs + 0x10, std::ios::beg);

        }

    }

    void CEntry::WriteEntry(std::fstream *file) {

        int tmp1, tmp2, tmp3;
        int currOffs = (int) file->tellg();

        if (m_Flags & 0x1)
            tmp1 = m_Index | (m_Id << 16);
        else
            tmp1 = m_Id;

        tmp2 = m_Length << 8;
        tmp2 |= m_Flags;

        file->write((char *) &tmp1, 4);
        file->write((char *) &tmp2, 4);
        file->write((char *) &m_Count, 4);

        if (m_Length == 0)
            tmp3 = ((m_RealOffs - currOffs) >> 4);
        else
            tmp3 = m_RealOffs - currOffs;

        file->write((char *) &tmp3, 4);

    }

    void CEntry::WriteData(std::fstream *file) {
        m_pData->Write(file);
    }

    // -- accessors --

    ENTRY_ID CEntry::GetId() {
        return m_Id;
    }

    int CEntry::GetIndex() {
        return m_Index;
    }

    int CEntry::GetLength() {
        return m_Length;
    }

    int CEntry::GetCount() {
        return m_Count;
    }

    int CEntry::GetFlags() {
        return m_Flags;
    }

    bool CEntry::IsDataEntry() {
        // REDEFINED!
        //return (m_Length!=0);
        return (m_Id != ID_ARTI);
    }

    ICrpData *CEntry::GetData() {
        if (this != NULL)
            return m_pData;
        else
            return NULL;
    }

    // -- modifiers --

    void CEntry::SetLength(int length) {
        m_Length = length;
    }

    void CEntry::SetCount(int count) {
        m_Count = count;
    }

    void CEntry::SetFlags(int flags) {
        m_Flags = flags;
    }

    void CEntry::SetData(ICrpData *pData) {
        if (m_pData != NULL) delete m_pData;
        m_pData = pData;
    }

    void CEntry::SetTargetOffs(int Offs) {
        m_RealOffs = Offs;
    }

    // -- worker methods --

    CEntry *CEntry::GetSubEntry(int index) {
        return (CEntry *) m_SubEntries[index];
    }

    std::string CEntry::GetEntryType() {
        char tmp[5];
        int id = (int) m_Id;
        tmp[3] = id & 0xFF;
        tmp[2] = (id >> 8) & 0xFF;
        tmp[1] = (id >> 16) & 0xFF;
        tmp[0] = (id >> 24) & 0xFF;
        if (tmp[0] == 0) {
            tmp[0] = tmp[2];
            tmp[1] = tmp[3];
            tmp[2] = 0;
        } else {
            tmp[4] = 0;
        }
        return std::string(tmp);
    }

    CEntry *CEntry::GetPartEntry(int Level, int PartIndex) {
        int FinIndex;
        FinIndex = (Level & 0xF) << 12;
        FinIndex |= (PartIndex & 0xFFF);

        return GetSubEntry(ID_PART, FinIndex);
    }

    CEntry *CEntry::GetDataEntry(ENTRY_ID Id, int Level, int AnimIndex, bool Damaged) {
        int FinIndex;
        FinIndex = (Level & 0xF);
        FinIndex |= ((AnimIndex & 0xFF) << 4);
        FinIndex |= (Damaged ? 0x8000 : 0);

        return GetSubEntry(Id, FinIndex);
    }

    CEntry *CEntry::GetSubEntry(ENTRY_ID Id, int Index) {

        if (!IsDataEntry()) {
            for (int i = 0; i < m_Count; i++) {
                CEntry *en = (CEntry *) m_SubEntries[i];
                if ((en->m_Id == Id) && (en->m_Index == Index))
                    return en;
            }
        }

        return NULL;

    }

    // -- new methods --

    // firstly a helper:
    void CEntry::InsertSubEntry(CEntry *newEn) {

        int Id = newEn->GetId();
        if (Id < 0x10000)
            Id = (Id << 16) | (newEn->GetIndex());

        bool inserted = false;

        for (int i = 0; i < m_Count; i++) {
            CEntry *en = (CEntry *) m_SubEntries[i];

            int enId = en->GetId();
            if (enId < 0x10000)
                enId = (enId << 16) | (en->GetIndex());

            if (enId > Id) {
                // insert before i
                inserted = true;
                m_SubEntries.insert(m_SubEntries.begin() + i, newEn);
                break;
            }
        }

        if (!inserted) {
            // last entry
            m_SubEntries.push_back(newEn);
        }

        m_Count++;

    }

    CEntry *CEntry::NewSubEntry(ENTRY_ID Id, int Index, bool Allocate) {
        if (!IsDataEntry()) {
            CEntry *newEn = new CEntry(Id, Index);
            int flags;

            switch (Id) {
                case ID_BASE:
                    flags = 0xEA;
                    break;
                case ID_NAME:
                    flags = 0xFA;
                    break;
                case ID_EFFECT:
                    flags = 0xF3;
                    break;
                default:
                    flags = 0xFA;
                    if ((int) Id < 0x10000) flags |= 0x01;
            }

            newEn->SetFlags(flags);

            if (Allocate)
                newEn->SetData(AllocateDataEntry(Id));

            InsertSubEntry(newEn);
            return newEn;
        } else {
            return NULL;
        }
    }

    CEntry *CEntry::NewDataEntry(ENTRY_ID Id, int Level, int AnimIndex, bool Damaged, bool Anim, bool Allocate) {
        if (!IsDataEntry()) {
            int Index;
            Index = (Level & 0xF);
            Index |= ((AnimIndex & 0xFF) << 4);
            Index |= (Damaged ? 0x8000 : 0);

            CEntry *newEn = new CEntry(Id, Index);
            int flags = 0x2;
            if ((int) Id < 0x10000) flags |= 0x01;
            if (Damaged || Anim) flags |= 0x80;
            flags |= ((Level & 0xF) << 3);

            newEn->SetFlags(flags);

            if (Allocate)
                newEn->SetData(AllocateDataEntry(Id));

            InsertSubEntry(newEn);
            return newEn;
        } else {
            return NULL;
        }
    }


    CEntry *CEntry::NewPartEntry(int Level, int PartIndex, bool Allocate) {
        if (!IsDataEntry()) {
            int Index;
            Index = (Level & 0xF) << 12;
            Index |= (PartIndex & 0xFFF);

            CEntry *newEn = new CEntry(ID_PART, Index);
            int flags = 0x3;
            flags |= ((Level & 0xF) << 3);

            newEn->SetFlags(flags);

            if (Allocate)
                newEn->SetData(AllocateDataEntry(ID_PART));

            InsertSubEntry(newEn);
            return newEn;
        } else {
            return NULL;
        }
    }
} // namespace