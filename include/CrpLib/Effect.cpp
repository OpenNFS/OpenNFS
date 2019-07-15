#include "Effect.h"

namespace CrpLib {

    CEffect::CEffect(void) {
        m_Unk1 = 0x5;
        m_Unk2 = 0x0;
    }

    CEffect::~CEffect(void) {

    }

    void CEffect::Read(std::fstream *file, ICrpEntry *entry) {
        file->read((char *) &m_Unk1, 4);
        file->read((char *) &m_Unk2, 4);

        /*
        CMatrix trTmp;
        trTmp.Read(file, NULL);

        // convert from Effect matrix to Standard matrix
        int i,j;
        for (i=0;i<4;i++)
            m_Tr.SetItem(3,i,trTmp.GetItem(0,i));
        for (i=0;i<3;i++)
            for (j=0;j<4;j++)
                m_Tr.SetItem(j,i,trTmp.GetItem(j+1,i));

        */

        int i, j;
        float fLine[4];
        file->read((char *) fLine, 4 * 4);
        for (i = 0; i < 4; i++)
            m_Tr.SetItem(3, i, fLine[i]);
        for (i = 0; i < 3; i++) {
            file->read((char *) fLine, 4 * 4);
            for (j = 0; j < 4; j++)
                m_Tr.SetItem(i, j, fLine[j]);
        }

        file->read((char *) &m_GlowColor, 4);
        file->read((char *) &m_SrcColor, 4);
        file->read((char *) &m_Mirror, 4);

        char Type;        // GLARE_TYPE
        char Extra;        // GLARE_EXTRA
        char Info;        // GLARE_INFO
        char Headlight;

        file->read((char *) &Type, 1);
        file->read((char *) &Extra, 1);
        file->read((char *) &Info, 1);
        file->read((char *) &Headlight, 1);

        m_GlareInfo.Type = (GLARE_TYPE) Type;
        m_GlareInfo.Extra = (GLARE_EXTRA) Extra;
        m_GlareInfo.Info = (GLARE_INFO) Info;
        m_GlareInfo.Headlight = (Headlight == 1);

    }

    void CEffect::Write(std::fstream *file) {

        file->write((char *) &m_Unk1, 4);
        file->write((char *) &m_Unk2, 4);

        /*
        CMatrix trTmp;

        // convert from Standard matrix to Effect matrix
        int i,j;
        for (i=0;i<4;i++)
            trTmp.SetItem(0,i,m_Tr.GetItem(3,i));
        for (i=0;i<3;i++)
            for (j=0;j<4;j++)
                trTmp.SetItem(j+1,i,trTmp.GetItem(j,i));

        trTmp.Write(file);
        */

        int i, j;
        float fLine[4];
        for (i = 0; i < 4; i++)
            fLine[i] = m_Tr.GetItem(3, i);
        file->write((char *) fLine, 4 * 4);

        for (i = 0; i < 3; i++) {
            for (j = 0; j < 4; j++)
                fLine[j] = m_Tr.GetItem(i, j);
            file->write((char *) fLine, 4 * 4);
        }

        file->write((char *) &m_GlowColor, 4);
        file->write((char *) &m_SrcColor, 4);
        file->write((char *) &m_Mirror, 4);

        char Type;        // GLARE_TYPE
        char Extra;        // GLARE_EXTRA
        char Info;        // GLARE_INFO
        char Headlight;

        Type = (char) m_GlareInfo.Type;
        Extra = (char) m_GlareInfo.Extra;
        Info = (char) m_GlareInfo.Info;
        Headlight = (m_GlareInfo.Headlight ? 1 : 0);

        file->write((char *) &Type, 1);
        file->write((char *) &Extra, 1);
        file->write((char *) &Info, 1);
        file->write((char *) &Headlight, 1);

    }

    int CEffect::GetEntryLength() {
        return 0x58;
    }

    int CEffect::GetEntryCount() {
        return 0x1;
    }

    // accessors

    int CEffect::GetGlowColor() {
        return m_GlowColor;
    }

    int CEffect::GetSourceColor() {
        return m_SrcColor;
    }

    bool CEffect::GetMirrored() {
        return (m_Mirror == 8);
    }

    tGlareInfo *CEffect::GetGlareInfo() {
        return &m_GlareInfo;
    }

    CMatrix *CEffect::GetTransform() {
        return &m_Tr;
    }

    // modifiers

    void CEffect::SetGlowColor(int value) {
        m_GlowColor = value;
    }

    void CEffect::SetSourceColor(int value) {
        m_SrcColor = value;
    }

    void CEffect::SetMirror(bool value) {
        m_Mirror = (value ? 8 : 0);
    }

    void CEffect::SetTransform(CMatrix *value) {
        m_Tr = *value;
    }


} // namespace