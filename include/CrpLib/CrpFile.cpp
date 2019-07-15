#include "CrpFile.h"

namespace CrpLib {
    CCrpFile::CCrpFile() {
        m_Offs = 0x10;
        m_ArtiCount = 0;
        m_MiscCount = 0;
        m_Flags = 0x1A;
        m_Id = ID_CAR;
    }

    CCrpFile::CCrpFile(std::string filename) {
        this->Open(filename);
    }

    CCrpFile::~CCrpFile(void) {
        int i;
        for (i = 0; i < m_ArtiCount; i++) {
            delete ((ICrpEntry *) m_Arti[i]);
        }
        for (i = 0; i < m_MiscCount; i++) {
            delete ((ICrpEntry *) m_Misc[i]);
        }
        m_Arti.clear();
        m_Misc.clear();
    }

    bool CCrpFile::Open(std::string filename) {
        std::fstream file(filename, std::ios::in | std::ios::binary);

        if(!file.is_open())
            return false;

        int tmp;
        file.read((char *) &tmp, 4);
        m_Id = (HEADER_ID) tmp;

        if (m_Id != ID_CAR && m_Id != ID_TRACK) {
            file.close();
            return false;
        }


        file.read((char *)&tmp, 4);
        m_Flags = tmp & 0x1F;
        m_ArtiCount = tmp >> 5;

        file.read((char *) &m_MiscCount, 4);

        file.read((char *) &m_Offs, 4);
        m_Offs <<= 4;

        file.seekg(m_Offs, std::ios::beg);

        ICrpEntry *en;

        m_Arti.reserve(m_ArtiCount);

        for (int i = 0; i < m_ArtiCount; i++) {
            en = new CEntry();
            en->Read(&file);
            m_Arti.push_back(en);
        }

        m_Misc.reserve(m_MiscCount);

        for (int i = 0; i < m_MiscCount; i++) {
            en = new CEntry();
            en->Read(&file);
            m_Misc.push_back(en);
        }

        file.close();

        return true;

    }

    bool CCrpFile::Save(std::string filename) {

#ifdef SAVE_COMPRESS
        CMemFile file(131072);
#else
        std::fstream file;
#endif

        int tmp, count, entryOffs, dataOffs;

        // verify count/length for all!
        for (int i = 0; i < m_ArtiCount; i++) {
            CEntry *arti = GetArticle(i);
            for (int j = 0; j < arti->GetCount(); j++) {
                CEntry *en = arti->GetSubEntry(j);
                ICrpData *data = en->GetData();
                if (data->GetEntryCount() != -1)
                    en->SetCount(data->GetEntryCount());
                if (data->GetEntryLength() != -1)
                    en->SetLength(data->GetEntryLength());
            }
        }
        for (int i = 0; i < m_MiscCount; i++) {
            CEntry *en = GetMisc(i);
            ICrpData *data = en->GetData();
            if (data->GetEntryCount() != -1)
                en->SetCount(data->GetEntryCount());
            if (data->GetEntryLength() != -1)
                en->SetLength(data->GetEntryLength());
        }


#ifndef SAVE_COMPRESS
        file.open(filename, std::ios::out | std::ios::binary);
        if(!file.is_open())
            return false;
#endif

#ifdef SAVE_PROTECT
        m_Offs = (m_ArtiCount+1) << 4;
#endif

        // write the header
#ifdef SAVE_PROTECT
        int newId = (int)m_Id ^ 0x20;
        file.Write(&newId, 4);
#else
        file.write((char *) &m_Id, 4);
#endif
        tmp = m_Flags | (m_ArtiCount << 5);
        file.write((char *) &tmp, 4);
        file.write((char *) &m_MiscCount, 4);
        tmp = m_Offs >> 4;
        file.write((char *) &tmp, 4);

#ifdef SAVE_PROTECT
        for	(int i=0; i<m_ArtiCount;i++) {
            tmp = ID_ARTI;	file.Write(&tmp,4);
            tmp = 0x1A;		file.Write(&tmp,4);
            tmp = 3*(i+1)*(m_MiscCount + m_ArtiCount)/2+5;
                file.Write(&tmp,4);
            tmp = ((m_MiscCount*3+m_ArtiCount+6)*43212)^(0x85850923*i);
            tmp &= (((i)<<5) - 1);
                file.Write(&tmp,4);
        }
#endif

        file.seekg(m_Offs, std::ios::beg);

        count = m_ArtiCount + m_MiscCount;
        for (int i = 0; i < m_ArtiCount; i++) count += ((CEntry *) m_Arti[i])->GetCount();

        dataOffs = m_Offs + count * 0x10 + 0x10;
        file.seekg(dataOffs, std::ios::beg);
        file.write(CRP_GENERATED_BY, (int) strlen(CRP_GENERATED_BY));

        dataOffs = ALIGN_OFFS((int) file.tellg(), 0x100);

        entryOffs = m_Offs + (m_ArtiCount + m_MiscCount) * 0x10;
        file.seekg(entryOffs, std::ios::beg);

        for (int i = 0; i < m_ArtiCount; i++) {
            CEntry *arti = (CEntry *) m_Arti[i];
            arti->SetTargetOffs(entryOffs);
            for (int j = 0; j < arti->GetCount(); j++) {
                CEntry *en = (CEntry *) (arti->GetSubEntry(j));

                en->SetTargetOffs(dataOffs);

                // write the data
                file.seekg(dataOffs, std::ios::beg);
                en->WriteData(&file);
                dataOffs = ALIGN_OFFS((int) file.tellg(), 0x10);

                // write the entry
                file.seekg(entryOffs, std::ios::beg);
                en->WriteEntry(&file);
                entryOffs += 0x10;

            }
            dataOffs = ALIGN_OFFS(dataOffs, 0x100);
        }

        file.seekg(m_Offs, std::ios::beg);

        for (int i = 0; i < m_ArtiCount; i++) {
            CEntry *arti = (CEntry *) m_Arti[i];
            arti->WriteEntry(&file);
        }

        entryOffs = (int) file.tellg();
        for (int i = 0; i < m_MiscCount; i++) {
            CEntry *en = (CEntry *) m_Misc[i];
            en->SetTargetOffs(dataOffs);

            file.seekg(dataOffs, std::ios::beg);
            en->WriteData(&file);
            dataOffs = ALIGN_OFFS((int) file.tellg(), 0x10);

            file.seekg(entryOffs, std::ios::beg);
            en->WriteEntry(&file);
            entryOffs += 0x10;
        }

        dataOffs = ALIGN_OFFS(dataOffs, 0x100);
        file.seekg(dataOffs - 1, std::ios::beg);
        file.write("\0", 1);

#ifndef SAVE_COMPRESS
        file.close();
#else
        unsigned int fLength = (unsigned int)file.GetLength();
        unsigned char* fData = file.Detach();
        CompressSave(fData, fLength, (char*)(LPCTSTR)filename);
#endif

        return true;

    }

    // -- accessors --

    int CCrpFile::GetArticleCount() {
        return m_ArtiCount;
    }

    CEntry *CCrpFile::GetArticle(int index) {
        if (index < m_ArtiCount)
            return (CEntry *) m_Arti[index];
        else
            return NULL;
    }

    int CCrpFile::GetMiscCount() {
        return m_MiscCount;
    }

    CEntry *CCrpFile::GetMisc(int index) {
        if (index < m_MiscCount)
            return (CEntry *) m_Misc[index];
        else
            return NULL;
    }

    CEntry *CCrpFile::GetMisc(ENTRY_ID id, int index) {
        for (int i = 0; i < m_MiscCount; i++) {
            CEntry *en = (CEntry *) m_Misc[i];
            if (en->GetId() == id && en->GetIndex() == index)
                return en;
        }
        return NULL;
    }

    int CCrpFile::FindMisc(ENTRY_ID id, int index) {

        for (int i = 0; i < m_MiscCount; i++) {
            CEntry *en = (CEntry *) m_Misc[i];
            if (en->GetId() == id && en->GetIndex() == index)
                return i;
        }
        return -1;

    }


    int CCrpFile::GetFlags() {
        return m_Flags;
    }

    void CCrpFile::SetFlags(int flags) {
        m_Flags = flags;
    }

    HEADER_ID CCrpFile::GetId() {
        return m_Id;
    }

    void CCrpFile::SetId(HEADER_ID id) {
        m_Id = id;
    }

    // new methods

    // firstly a helper:
    void CCrpFile::InsertMiscEntry(CEntry *newEn) {
        int Id = newEn->GetId();
        if (Id < 0x10000)
            Id = (Id << 16) | (newEn->GetIndex());

        bool inserted = false;

        for (int i = 0; i < m_MiscCount; i++) {
            CEntry *en = (CEntry *) m_Misc[i];
            int enId = en->GetId();
            if (enId < 0x10000)
                enId = (enId << 16) | (en->GetIndex());

            if (enId > Id) {
                // insert before i
                inserted = true;
                m_Misc.insert(m_Misc.begin() + i, newEn);
                break;
            }
        }

        if (!inserted) {
            // last entry
            m_Misc.push_back(newEn);
        }

        m_MiscCount++;
    }

    CEntry *CCrpFile::NewArticle(void) {
        CEntry *en = new CEntry(ID_ARTI, 0);
        en->SetFlags(0x1A);
        m_Arti.push_back(en);
        m_ArtiCount++;
        return en;
    }

    CEntry *CCrpFile::NewMisc(ENTRY_ID id, int index, bool Allocate) {
        CEntry *en = new CEntry(id, index);

        int flags = 0xFA;
        if ((int) id < 0x10000) flags |= 0x01;
        en->SetFlags(flags);

        if (Allocate){
            en->SetData(AllocateDataEntry(id));
        }

        InsertMiscEntry(en);

        return en;
    }

} // namespace

