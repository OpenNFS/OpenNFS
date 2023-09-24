#include "Vector2.h"

namespace CrpLib {

    void CVector2::FreeData() {
        if (m_Init)
            delete[] m_pData;
    }


    CVector2::CVector2(void) {
        m_Init = false;
    }

    CVector2::CVector2(tVector2 *pData, int count) {
        m_Init = true;
        m_pData = new tVector2[count];
        memcpy(m_pData, pData, count * sizeof(tVector2));
        m_Count = count;
    }

    CVector2::~CVector2(void) {
        FreeData();
    }

    void CVector2::Read(std::fstream *file, ICrpEntry *entry) {
        FreeData();
        m_Init = true;

        CEntry *en = (CEntry *) entry;
        m_Count = en->GetCount();
        m_pData = new tVector2[m_Count];
        file->read((char *) m_pData, m_Count * sizeof(tVector2));
    }

    void CVector2::Write(std::fstream *file) {
        file->write((char *) m_pData, m_Count * sizeof(tVector2));
    }

    int CVector2::GetEntryLength() {
        return m_Count * 0x8;
    }

    int CVector2::GetEntryCount() {
        return m_Count;
    }

    // accessors and modifiers

    void CVector2::SetItem(int index, tVector2 *value) {
        m_pData[index] = *value;
    }

    tVector2 *CVector2::GetItem(int index) {
        return &m_pData[index];
    }

    int CVector2::GetCount() {
        return m_Count;
    }

    void CVector2::SetCount(int count) {
        FreeData();
        m_Init = true;
        m_pData = new tVector2[count];
        m_Count = count;
    }


} // namespace