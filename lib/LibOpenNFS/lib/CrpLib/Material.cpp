#include "Material.h"

namespace CrpLib {

    CMaterial::CMaterial(void) {

    }

    CMaterial::~CMaterial(void) {

    }

    void CMaterial::Read(std::fstream *file, ICrpEntry *entry) {
        file->read((char *) m_pData1, 0x10);
        file->read((char *) m_pRMthName, 0x10);
        file->read((char *) m_pData2, 0x8);
        file->read((char *) &m_TpgIndex, 4);
        file->read((char *) m_pData3, 0x10C);
    }


    void CMaterial::Write(std::fstream *file) {
        file->write((char *) m_pData1, 0x10);
        file->write((char *) m_pRMthName, 0x10);
        file->write((char *) m_pData2, 0x8);
        file->write((char *) &m_TpgIndex, 4);
        file->write((char *) m_pData3, 0x10C);
    }

    int CMaterial::GetEntryLength() {
        return 0x138;
    }

    int CMaterial::GetEntryCount() {
        return 0x34;
    }

    int CMaterial::GetTpgIndex() {
        return m_TpgIndex;
    }

    void CMaterial::SetTpgIndex(int index) {
        m_TpgIndex = index;
    }

    void CMaterial::SetCull(bool cull) {
        if (cull) {
            m_pData1[0xC] = 0x20;
            m_pData3[0xE4] = 0x1;
        } else {
            m_pData1[0xC] = 0x0;
            m_pData3[0xE4] = 0x0;
        }
    }

    char *CMaterial::GetRenderMethodName() {
        return m_pRMthName;
    }

}