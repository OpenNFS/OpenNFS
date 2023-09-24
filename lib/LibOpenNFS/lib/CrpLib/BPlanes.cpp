#include "BPlanes.h"


namespace CrpLib {

    void CBPlanes::FreeData() {
        if (m_Init) {
            delete[] m_pVertices;
        }
    }

    CBPlanes::CBPlanes(void) {
        m_Init = false;
        m_Count = 0;
        m_Unk1 = m_Unk2 = 0;
    }

    CBPlanes::~CBPlanes(void) {
        FreeData();
    }

    void CBPlanes::Read(std::fstream *file, ICrpEntry *entry) {

        CEntry *en = (CEntry *) entry;
        m_Count = en->GetCount() / 4;

        FreeData();
        m_Init = true;

        file->read((char *) &m_Unk1, 4);
        file->read((char *) &m_Unk2, 4);

        m_pVertices = new tVector4[m_Count * 4];
        file->read((char *) m_pVertices, m_Count * 4 * sizeof(tVector4));

    }

    void CBPlanes::Write(std::fstream *file) {

        file->write((char *) &m_Unk1, 4);
        file->write((char *) &m_Unk2, 4);

        file->write((char *) m_pVertices, m_Count * 4 * sizeof(tVector4));

    }

    int CBPlanes::GetEntryLength() {
        return (0x8 + 0x10 * m_Count * 4);
    }

    int CBPlanes::GetEntryCount() {
        return (m_Count * 4);
    }


    int CBPlanes::GetCount() {
        return m_Count;
    }

    void CBPlanes::SetCount(int value) {
        if (m_Init) delete[] m_pVertices;
        m_Init = true;
        m_pVertices = new tVector4[value * 4];
        m_Count = value;
    }

    tVector4 *CBPlanes::GetVertices() {
        return m_pVertices;
    }

}