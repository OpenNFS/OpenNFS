#include "Matrix.h"

namespace CrpLib {

    CMatrix::CMatrix(void) {
        for (int i = 0; i < 16; i++)
            m_Items[i] = (i % 5 == 0) ? 1.0f : 0.0f;
    }

    CMatrix::CMatrix(float *values) {
        memcpy(m_Items, values, 16 * 4);
    }

    CMatrix::~CMatrix(void) {

    }


    void CMatrix::Read(std::fstream *file, ICrpEntry *entry) {
        file->read((char *) m_Items, 16 * 4);
    }

    void CMatrix::Write(std::fstream *file) {
        file->write((char *) m_Items, 16 * 4);
    }

    int CMatrix::GetEntryLength() {
        return 0x40;
    }

    int CMatrix::GetEntryCount() {
        return 0x1;
    }

    // accessors and modifiers

    float CMatrix::GetItem(int row, int col) {
        return m_Items[row * 4 + col];
    }

    void CMatrix::SetItem(int row, int col, float value) {
        m_Items[row * 4 + col] = value;
    }

    float *CMatrix::GetValues(){
        return m_Items;
    }

    void CMatrix::SetValues(float *values) {
        std::memcpy(m_Items, values, 4 * 16);
    }

} // namespace