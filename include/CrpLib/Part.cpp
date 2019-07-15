#include "Part.h"

namespace CrpLib {

    void CPart::FreeData() {
        if (m_Init) {
            delete[] m_pInfo;
            delete[] m_pIndex;
            delete[] m_pIndices;
        }
    }

    CPart::CPart(void) {
        m_Unk1 = (short) 0x8000;
        m_pIndex = NULL;
        m_pIndices = NULL;
        m_pInfo = NULL;
        m_Init = false;
    }

    CPart::~CPart(void) {
        FreeData();
    }

    void CPart::Read(std::fstream *file, ICrpEntry *entry) {

        CEntry *en = (CEntry *) entry;

        FreeData();
        m_Init = true;

        file->read((char *) &m_FillMode, 2);
        file->read((char *) &m_TransInfo, 2);

        file->read((char *) &m_Mat, 2);
        file->read((char *) &m_Unk1, 2);

        file->read((char *) &m_BSphere, sizeof(tVector4));
        file->read((char *) &m_Unk2, sizeof(tVector4));

        file->read((char *) &m_InfoCount, 4);
        file->read((char *) &m_IndexCount, 4);

        m_pInfo = new tPartInfo[m_InfoCount];
        m_pIndex = new tPartIndex[m_IndexCount];

        file->read((char *) m_pInfo, sizeof(tPartInfo) * m_InfoCount);
        file->read((char *) m_pIndex, sizeof(tPartIndex) * m_IndexCount);

        m_IndiceCount = m_IndexCount * en->GetCount();
        m_pIndices = new unsigned char[m_IndiceCount];
        file->read((char *) m_pIndices, m_IndiceCount);

    }

    void CPart::Write(std::fstream *file) {

        file->write((char *) &m_FillMode, 2);
        file->write((char *) &m_TransInfo, 2);

        file->write((char *) &m_Mat, 2);
        file->write((char *) &m_Unk1, 2);

        file->write((char *) &m_BSphere, sizeof(tVector4));
        file->write((char *) &m_Unk2, sizeof(tVector4));

        file->write((char *) &m_InfoCount, 4);
        file->write((char *) &m_IndexCount, 4);

        file->write((char *) m_pInfo, sizeof(tPartInfo) * m_InfoCount);
        file->write((char *) m_pIndex, sizeof(tPartIndex) * m_IndexCount);

        file->write((char *) m_pIndices, m_IndiceCount);

    }

    int CPart::GetEntryLength() {
        return 0x30 + 0x10 * m_InfoCount + 0x8 * m_IndexCount + m_IndiceCount;
    }

    int CPart::GetEntryCount() {
        return m_IndiceCount / m_IndexCount;
    }

    // inits an empty CPart with required Info and Index entries
    void CPart::InitStorage(ICrpEntry *entry, int indiceCount, RM_NAME rm) {

        FreeData();
        m_Init = true;

        CEntry *en = (CEntry *) entry;
        en->SetCount(indiceCount);

        m_InfoCount = 4;
        m_IndexCount = 2;

        m_pInfo = new tPartInfo[m_InfoCount];
        m_pIndex = new tPartIndex[m_IndexCount];

        m_pIndex[0].Id = (short) ID_INDEX_VERTEX;
        m_pIndex[0].Index = 1;

        m_pIndex[1].Id = (short) ID_INDEX_UV;
        m_pIndex[1].Index = 1;

        m_pInfo[0].Id = (short) ID_INFO_CULL;
        m_pInfo[0].IndexRowRef = -1;

        m_pInfo[1].Id = (short) ID_INFO_NORMAL;
        m_pInfo[1].IndexRowRef = 0;

        m_pInfo[2].Id = (short) ID_INFO_UV;
        m_pInfo[2].IndexRowRef = 1;

        m_pInfo[3].Id = (short) ID_INFO_VERTEX;
        m_pInfo[3].IndexRowRef = 0;

        switch (rm) {
            case RM_EXT:
                m_pInfo[0].RMOffs = 0x2;
                m_pInfo[1].RMOffs = 0x8;
                m_pInfo[2].RMOffs = 0x9;
                m_pInfo[3].RMOffs = 0xA;
                break;
            case RM_EXT_ENV:
                m_pInfo[0].RMOffs = 0x4;
                m_pInfo[1].RMOffs = 0x13;
                m_pInfo[2].RMOffs = 0x14;
                m_pInfo[3].RMOffs = 0x15;
                break;
            case RM_INT:
                m_pInfo[0].RMOffs = 0x1;
                m_pInfo[1].RMOffs = 0x7;
                m_pInfo[2].RMOffs = 0x8;
                m_pInfo[3].RMOffs = 0x9;
                break;
            case RM_WHEEL:
                m_pInfo[0].RMOffs = 0x1;
                m_pInfo[1].RMOffs = 0x7;
                m_pInfo[2].RMOffs = 0x8;
                m_pInfo[3].RMOffs = 0x9;
                break;
            case RM_WINDOW:
                m_pInfo[0].RMOffs = 0x4;
                m_pInfo[1].RMOffs = 0xE;
                m_pInfo[2].RMOffs = 0xF;
                m_pInfo[3].RMOffs = 0x10;
                break;
        }

        m_IndiceCount = indiceCount * m_IndexCount;
        m_pIndices = new unsigned char[m_IndiceCount];
        memset(m_pIndices,0,  m_IndiceCount);

    }

    short CPart::GetFillMode() {
        return m_FillMode;
    }

    void CPart::SetFillMode(short value) {
        m_FillMode = value;
    }

    PART_TRANS CPart::GetTransInfo() {
        return (PART_TRANS) m_TransInfo;
    }

    void CPart::SetTransInfo(PART_TRANS value) {
        m_TransInfo = (short) value;
    }

    short CPart::GetMaterial() {
        return m_Mat;
    }

    void CPart::SetMaterial(short value) {
        m_Mat = value;
    }

    tVector4 *CPart::GetBoundingSphere() {
        return &m_BSphere;
    }

    int CPart::GetInfoCount() {
        return m_InfoCount;
    }

    int CPart::GetIndexCount() {
        return m_IndexCount;
    }

    tPartInfo *CPart::GetInfo(int index) {
        return &m_pInfo[index];
    }

    tPartIndex *CPart::GetIndex(int index) {
        return &m_pIndex[index];
    }

    unsigned char *CPart::GetIndices(int index) {
        return &m_pIndices[m_pIndex[index].Offset];
    }

    int CPart::FindInfo(INFOROW_ID ird) {
        for (int i = 0; i < m_InfoCount; i++)
            if (m_pInfo[i].Id == (short) ird) return i;
        return -1;
    }

    int CPart::FindIndex(INDEXROW_ID ird) {
        for (int i = 0; i < m_IndexCount; i++)
            if (m_pIndex[i].Id == (short) ird) return i;
        return -1;
    }


} // namespace