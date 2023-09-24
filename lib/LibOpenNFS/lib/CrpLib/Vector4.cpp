#include "Vector4.h"

namespace CrpLib {

    void CVector4::FreeData() {
        if (m_Init)
            delete[] m_pData;
    }

    CVector4::CVector4(void) {
        m_Init = false;
    }

    CVector4::CVector4(tVector4 *pData, int count) {
        m_Init = true;
        m_pData = new tVector4[count];
        memcpy(m_pData, pData, count * sizeof(tVector4));
        m_Count = count;
    }

    CVector4::~CVector4(void) {
        FreeData();
    }

    void CVector4::Read(std::fstream *file, ICrpEntry *entry) {
        FreeData();
        m_Init = true;

        CEntry *en = (CEntry *) entry;
        m_Count = en->GetCount();
        m_pData = new tVector4[m_Count];
        file->read((char *) m_pData, m_Count * sizeof(tVector4));
    }

    void CVector4::Write(std::fstream *file) {
        file->write((char *) m_pData, m_Count * sizeof(tVector4));
    }

    int CVector4::GetEntryLength() {
        return m_Count * 0x10;
    }

    int CVector4::GetEntryCount() {
        return m_Count;
    }

    // accessors and modifiers

    void CVector4::SetItem(int index, tVector4 *value) {
        m_pData[index] = *value;
    }

    tVector4 *CVector4::GetItem(int index) {
        return &m_pData[index];
    }

    int CVector4::GetCount() {
        return m_Count;
    }

    void CVector4::SetCount(int count) {
        FreeData();
        m_Init = true;
        m_pData = new tVector4[count];
        m_Count = count;
    }


} // namespace