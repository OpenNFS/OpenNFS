#pragma once

#include "Interfaces.h"
#include "Entry.h"
#include "Common.h"

#include <cstring>
#include <fstream>

/*

	NOTE: CEntry->Count is number of indices
			NOT number of triangles

*/

namespace CrpLib {

    class CPart :
            public ICrpData {
    private:

        short m_FillMode;        // D3DFILLMODE
        short m_TransInfo;        // PART_TRANS

        short m_Mat;
        short m_Unk1;

        tVector4 m_BSphere;
        tVector4 m_Unk2;

        int m_InfoCount;
        int m_IndexCount;

        tPartInfo *m_pInfo;
        tPartIndex *m_pIndex;
        unsigned char *m_pIndices;
        int m_IndiceCount;            // total count for write
        bool m_Init;

        // clean up helper
        void FreeData();

    public:

        CPart(void);

        ~CPart(void);

        void Read(std::fstream *file, ICrpEntry *entry);

        void Write(std::fstream *file);

        int GetEntryLength();

        int GetEntryCount();

        // inits an empty CPart with required Info and Index entries
        void InitStorage(ICrpEntry *entry, int indiceCount, RM_NAME rm);

        short GetFillMode();

        void SetFillMode(short value);

        PART_TRANS GetTransInfo();

        void SetTransInfo(PART_TRANS value);

        short GetMaterial();

        void SetMaterial(short value);

        tVector4 *GetBoundingSphere();

        int GetInfoCount();

        int GetIndexCount();

        tPartInfo *GetInfo(int index);

        tPartIndex *GetIndex(int index);

        unsigned char *GetIndices(int index);

        int FindInfo(INFOROW_ID ird);

        int FindIndex(INDEXROW_ID ird);

    };

}
