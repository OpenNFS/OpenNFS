#pragma once

#include "Interfaces.h"
#include "Matrix.h"
#include "Common.h"

#include <fstream>

namespace CrpLib {

    class CEffect :
            public ICrpData {
    private:
        int m_Unk1; // 0x5 always
        int m_Unk2; // 0x0 always

        CMatrix m_Tr;

        int m_GlowColor, m_SrcColor;
        int m_Mirror;

        tGlareInfo m_GlareInfo;

    public:

        CEffect(void);

        ~CEffect(void);

        void Read(std::fstream *file, ICrpEntry *entry);

        void Write(std::fstream *file);

        int GetEntryLength();

        int GetEntryCount();

        // accessors

        int GetGlowColor();

        int GetSourceColor();

        bool GetMirrored();

        tGlareInfo *GetGlareInfo();

        CMatrix *GetTransform();

        // modifiers

        void SetGlowColor(int value);

        void SetSourceColor(int value);

        void SetMirror(bool value);

        void SetTransform(CMatrix *value);

    };

}
