#include "Base.h"

namespace CrpLib {

    void CBase::FreeData() {
        if (m_Init) {
            delete[] m_pLevMasks;
        }
    }

    CBase::CBase(void) {
        m_Init = false;
        m_Flags = 0;
        m_Zero1 = m_Zero2 = 0;
        m_LevelCount = 0;
        m_HasFloats = 0;
        memset(m_pFloats, 0, 16 * 4);
        m_BaseInfo = tBaseInfo();
    }

    CBase::~CBase(void) {
        FreeData();
    }

    void CBase::Read(std::fstream *file, ICrpEntry *entry) {

        FreeData();
        m_Init = true;

        file->read((char *) &m_Flags, 4);
        file->read((char *) &m_Zero1, 4);
        file->read((char *) &m_Zero2, 4);
        file->read((char *) &m_LevelCount, 4);

        file->read((char *) &m_HasFloats, 4);
        file->read((char *) m_pFloats, 12 * 4);

        file->read((char *) &m_BaseInfo, sizeof(tBaseInfo));

        file->read((char *) &m_Unk, sizeof(tVector4));

        m_pLevMasks = new tLevelMask[m_LevelCount];
        file->read((char *) m_pLevMasks, sizeof(tLevelMask) * m_LevelCount);


    }

    void CBase::Write(std::fstream *file) {

        file->write((char *) &m_Flags, 4);
        file->write((char *) &m_Zero1, 4);
        file->write((char *) &m_Zero2, 4);
        file->write((char *) &m_LevelCount, 4);

        file->write((char *) &m_HasFloats, 4);
        file->write((char *) m_pFloats, 12 * 4);

        file->write((char *) &m_BaseInfo, sizeof(tBaseInfo));

        file->write((char *) &m_Unk, sizeof(tVector4));

        file->write((char *) m_pLevMasks, sizeof(tLevelMask) * m_LevelCount);

    }

    int CBase::GetEntryLength() {
        return (0x64 + m_LevelCount * 0xC);
    }

    int CBase::GetEntryCount() {
        return 0x0;
    }

    void CBase::CreateLevels(int LevCount, int Start) {

        if (m_Init)
            delete[] m_pLevMasks;

        m_LevelCount = LevCount;
        m_pLevMasks = new tLevelMask[m_LevelCount];

        for (int i = 0; i < LevCount; i++) {
            m_pLevMasks[i].SetLevel(i + Start);
        }

    }

    int CBase::GetLevelCount() {
        return m_LevelCount;
    }

    tLevelMask *CBase::GetLevel(int i) {
        return &m_pLevMasks[i];
    }

    BASE_FLAGS CBase::GetFlags() {
        return (BASE_FLAGS) m_Flags;
    }

    void CBase::SetFlags(BASE_FLAGS flags) {
        m_Flags = (int) flags;
        if (flags == BF_EFFECT) {
            m_HasFloats = 1;
        } else {
            m_HasFloats = 0;
        }
    }

    void CBase::SetHasFloats(bool val) {
        m_HasFloats = (val ? 1 : 0);
    }

    float *CBase::GetFloats() {
        if (m_HasFloats == 1) {
            return m_pFloats;
        } else {
            return NULL;
        }
    }

    tBaseInfo *CBase::GetBaseInfo() {
        return &m_BaseInfo;
    }

} // namespace