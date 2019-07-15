#include "RawData.h"

namespace CrpLib {

    void CRawData::FreeData() {
        if (m_Init)
            delete[] m_pData;
    }

    CRawData::CRawData(void) {
        m_Init = false;
        m_pData = NULL;
        m_Length = NULL;
    }

    CRawData::CRawData(char *pData, int length) {
        m_Init = false;
        SetData(pData, length);
    }

    CRawData::~CRawData(void) {
        FreeData();
    }

    void CRawData::Read(std::fstream *file, ICrpEntry *entry) {
        FreeData();
        m_Init = true;

        CEntry *en = (CEntry *) entry;
        m_Length = en->GetLength();
        m_pData = new char[m_Length];
        file->read((char *) m_pData, m_Length);
    }

    void CRawData::Write(std::fstream *file) {
        file->write((char *) m_pData, m_Length);
    }

    int CRawData::GetEntryLength() {
        return m_Length;
    }

    int CRawData::GetEntryCount() {
        return -1;
    }


    bool CRawData::ReadFromFile(std::string filename) {
        bool ret = false;
        std::ifstream *file = new ifstream(filename, std::ios::in | std::ios::binary);
        if (file->is_open()) {
            m_Length = file->tellg();
            file->seekg( 0, std::ios::end );
            m_Length = file->tellg() - (std::streamoff) m_Length;
            // Rewind after getting length
            file->seekg(0, std::ios::beg);

            FreeData();
            m_Init = true;
            m_pData = new char[m_Length];
            file->read((char *) m_pData, m_Length);

            file->close();
            delete file;

            ret = true;

        }

        return ret;

    }

    bool CRawData::WriteToFile(std::string filename) {
        bool ret = false;
        std::ofstream *file = new ofstream(filename, std::ios::out | std::ios::binary);

        if (file->is_open()) {
            file->write((char *) m_pData, m_Length);
            file->close();
            delete file;
            ret = true;
        }
        return ret;
    }


    /*void CRawData::ParseFrom(ICrpEntry *entry) {
        CMemFile *mfile = new CMemFile();
        mfile->SetLength(((CEntry *) entry)->GetLength());

        ICrpData *data = ((CEntry *) entry)->GetData();

        data->Write(mfile);

        mfile->Seek(0, CMemFile::begin);

        this->Read(mfile, entry);

        delete mfile;

    }

    void CRawData::ParseTo(ICrpEntry *entry) {

        CMemFile *mfile = new CMemFile();
        mfile->SetLength(this->m_Length);

        ICrpData *data = ((CEntry *) entry)->GetData();

        this->Write(mfile);

        mfile->Seek(0, CMemFile::begin);

        data->Read(mfile, entry);

        delete mfile;
    }*/


    // accessors and modifiers

    void CRawData::SetData(char *pData, int length) {
        FreeData();
        m_Init = true;
        m_pData = new char[length];
        memcpy(m_pData, pData, length);
        m_Length = length;
    }

    void CRawData::SetData(std::string data) {
        this->SetData((char *) data.c_str(), data.length() + 1);
    }


    char *CRawData::GetData() {
        return m_pData;
    }

    int CRawData::GetLength() {
        return m_Length;
    }


} // namespace