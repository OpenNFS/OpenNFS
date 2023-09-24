#pragma once

#include "Interfaces.h"

#include <cstring>
#include <fstream>

namespace CrpLib {

    class CMatrix :
            public ICrpData {
    private:
        float m_Items[16];

    public:

        CMatrix(void);

        CMatrix(float *values);

        ~CMatrix(void);

        void Read(std::fstream *file, ICrpEntry *entry);

        void Write(std::fstream *file);

        int GetEntryLength();

        int GetEntryCount();

        // accessors and modifiers

        float GetItem(int row, int col);

        void SetItem(int row, int col, float value);

        float *GetValues();

        void SetValues(float *values);

    };

}
