//
// Created by Amrik on 21/08/2019.
//

#include <cstdint>
#include "SerializedGroupOps.h"

#define this test

SerializedGroup *LocateGroupType__15SerializedGroupii(SerializedGroup *this, int type, int index) {
    /*  bool bVar1;
      uint8_t uVar2;
      int iVar3;
      int iVar4;

      iVar4 = 0;
      iVar3 = this->m_num_elements;
      this = this + 1;
      while (true) {
          iVar3 = iVar3 + -1;
          if (iVar3 == -1) {
              return (SerializedGroup *) 0x0;
          }
          if ((this->m_type == type) && (bVar1 = iVar4 == index, iVar4 = iVar4 + 1, bVar1)) break;
          uVar2 = this->m_length & 3;
          if (uVar2 != 0) {
              this->m_length = (this->m_length + 4U) - uVar2;
          }
          this = (SerializedGroup *) ((int) &this->m_type + this->m_length);
      }*/
    return this;
}

/*Group *CreateLiteGroup__15SerializedGroupP15SerializedGroupP9SimpleMem(SerializedGroup *this, SerializedGroup *source, SimpleMem *mem) {
    Group *pGVar1;
    int len;

    len = source->m_length + -0xc;
    pGVar1 = (Group *) Alloc__9SimpleMemii(mem, len, 0);
    pGVar1->m_num_elements = source->m_num_elements;
    blockmove((undefined4 * )(source + 1), pGVar1 + 1, len);
    return pGVar1;
}*/

/*void blockmove(undefined4 *param_1,undefined4 *param_2,int param_3)

{
    undefined *puVar1;
    uint uVar2;
    uint uVar3;
    int iVar4;
    undefined4 uVar5;
    undefined4 uVar6;
    undefined4 uVar7;
    undefined4 uVar8;
    undefined4 uVar9;
    undefined4 uVar10;
    undefined4 uVar11;

    if (((int)param_1 < (int)param_2) && ((int)param_2 < (int)param_1 + param_3)) {
        uVar2 = (int)param_1 + param_3;
        uVar3 = (int)param_2 + param_3;
        if (((uVar2 | uVar3) & 3) == 0) {
            param_3 = param_3 + -0x10;
            while (-1 < param_3) {
                uVar5 = *(undefined4 *)(uVar2 - 0xc);
                uVar6 = *(undefined4 *)(uVar2 - 8);
                uVar7 = *(undefined4 *)(uVar2 - 4);
                *(undefined4 *)(uVar3 - 0x10) = *(undefined4 *)(uVar2 - 0x10);
                *(undefined4 *)(uVar3 - 0xc) = uVar5;
                *(undefined4 *)(uVar3 - 8) = uVar6;
                *(undefined4 *)(uVar3 - 4) = uVar7;
                uVar2 = uVar2 - 0x10;
                param_3 = param_3 + -0x10;
                uVar3 = uVar3 - 0x10;
            }
        }
        else {
            param_3 = param_3 + -0x10;
            while (-1 < param_3) {
                uVar5 = *(undefined4 *)(uVar2 - 0xc);
                uVar6 = *(undefined4 *)(uVar2 - 8);
                uVar7 = *(undefined4 *)(uVar2 - 4);
                *(undefined4 *)(uVar3 - 0x10) = *(undefined4 *)(uVar2 - 0x10);
                *(undefined4 *)(uVar3 - 0xc) = uVar5;
                *(undefined4 *)(uVar3 - 8) = uVar6;
                *(undefined4 *)(uVar3 - 4) = uVar7;
                param_3 = param_3 + -0x10;
                uVar2 = uVar2 - 0x10;
                uVar3 = uVar3 - 0x10;
            }
        }
        param_3 = param_3 + 0xc;
        while (-1 < param_3) {
            param_3 = param_3 + -4;
            *(undefined4 *)(uVar3 - 4) = *(undefined4 *)(uVar2 - 4);
            uVar2 = uVar2 - 4;
            uVar3 = uVar3 - 4;
        }
        iVar4 = param_3 + 2;
        if (-1 < param_3 + 3) {
            while( true ) {
                puVar1 = (undefined *)(uVar2 - 1);
                uVar2 = uVar2 - 1;
                *(undefined *)(uVar3 - 1) = *puVar1;
                uVar3 = uVar3 - 1;
                if (iVar4 < 0) break;
                iVar4 = iVar4 + -1;
            }
        }
        return;
    }
    if ((((uint)param_1 | (uint)param_2) & 3) != 0) {
        param_3 = param_3 + -0x10;
        while (-1 < param_3) {
            uVar5 = param_1[1];
            uVar6 = param_1[2];
            uVar7 = param_1[3];
            *param_2 = *param_1;
            param_2[1] = uVar5;
            param_2[2] = uVar6;
            param_2[3] = uVar7;
            param_3 = param_3 + -0x10;
            param_1 = param_1 + 4;
            param_2 = param_2 + 4;
        }
        param_3 = param_3 + 0xc;
        while (-1 < param_3) {
            param_3 = param_3 + -4;
            *param_2 = *param_1;
            param_1 = param_1 + 1;
            param_2 = param_2 + 1;
        }
        param_3 = param_3 + 3;
        while (-1 < param_3) {
            param_3 = param_3 + -1;
            *(undefined *)param_2 = *(undefined *)param_1;
            param_1 = (undefined4 *)((int)param_1 + 1);
            param_2 = (undefined4 *)((int)param_2 + 1);
        }
        return;
    }
    param_3 = param_3 + -0x40;
    while (-1 < param_3) {
        uVar5 = param_1[1];
        uVar6 = param_1[2];
        uVar7 = param_1[3];
        uVar8 = param_1[4];
        uVar9 = param_1[5];
        uVar10 = param_1[6];
        uVar11 = param_1[7];
        *param_2 = *param_1;
        param_2[1] = uVar5;
        param_2[2] = uVar6;
        param_2[3] = uVar7;
        param_2[4] = uVar8;
        param_2[5] = uVar9;
        param_2[6] = uVar10;
        param_2[7] = uVar11;
        uVar5 = param_1[9];
        uVar6 = param_1[10];
        uVar7 = param_1[0xb];
        uVar8 = param_1[0xc];
        uVar9 = param_1[0xd];
        uVar10 = param_1[0xe];
        uVar11 = param_1[0xf];
        param_2[8] = param_1[8];
        param_2[9] = uVar5;
        param_2[10] = uVar6;
        param_2[0xb] = uVar7;
        param_2[0xc] = uVar8;
        param_2[0xd] = uVar9;
        param_2[0xe] = uVar10;
        param_2[0xf] = uVar11;
        param_3 = param_3 + -0x40;
        param_1 = param_1 + 0x10;
        param_2 = param_2 + 0x10;
    }
    param_3 = param_3 + 0x30;
    while (-1 < param_3) {
        uVar5 = param_1[1];
        uVar6 = param_1[2];
        uVar7 = param_1[3];
        *param_2 = *param_1;
        param_2[1] = uVar5;
        param_2[2] = uVar6;
        param_2[3] = uVar7;
        param_3 = param_3 + -0x10;
        param_1 = param_1 + 4;
        param_2 = param_2 + 4;
    }
    param_3 = param_3 + 0xc;
    while (-1 < param_3) {
        param_3 = param_3 + -4;
        *param_2 = *param_1;
        param_1 = param_1 + 1;
        param_2 = param_2 + 1;
    }
    param_3 = param_3 + 3;
    while (-1 < param_3) {
        param_3 = param_3 + -1;
        *(undefined *)param_2 = *(undefined *)param_1;
        param_1 = (undefined4 *)((int)param_1 + 1);
        param_2 = (undefined4 *)((int)param_2 + 1);
    }
    return;
}*/

/*Group *LocateCreateGroupType__15SerializedGroupiP9SimpleMemi(SerializedGroup *this, int type, SimpleMem *mem, int index) {
    SerializedGroup *source;
    Group *pGVar1;

    source = LocateGroupType__15SerializedGroupii(this, type, index);
    if (source == (SerializedGroup *) 0x0) {
        pGVar1 = (Group *) 0x0;
    } else {
        source = LocateGroupType__15SerializedGroupii(this, type, index);
        pGVar1 = CreateLiteGroup__15SerializedGroupP15SerializedGroupP9SimpleMem(this, source, mem);
    }
    return pGVar1;
}*/

/*
void InstanceGroup__5ChunkP15SerializedGroupP9SimpleMem(Chunk *this, SerializedGroup *chunkGroup, SimpleMem *mem){
    char cVar1;
    SerializedGroup *source;
    Group *pGVar2;
    SerializedGroup *source_00;
    uint8_t uVar3;
    SerializedGroup *pSVar4;
    SerializedGroup *pSVar5;
    int iVar6;
    int iVar7;
    int dataSize;

    source = LocateGroupType__15SerializedGroupii(chunkGroup,0x1c,0);
    this->firstSimSliceInd = *(short *)((int)&source[1].dummy + 2);
    this->boundPts[0] = *(RelCoord16 *)&source[2].m_type;
    this->boundPts[1] = *(RelCoord16 *)&source[2].m_length;
    this->boundPts[2] = *(RelCoord16 *)&source[2].dummy;
    this->boundPts[3] = *(RelCoord16 *)&source[2].m_num_elements;
    this->chunkboundPts[0] = *(RelCoord16 *)&source[3].m_type;
    this->chunkboundPts[1] = *(RelCoord16 *)&source[3].m_length;
    this->chunkboundPts[2] = *(RelCoord16 *)&source[3].dummy;
    this->chunkboundPts[3] = *(RelCoord16 *)&source[3].m_num_elements;
    this->chunkInd = *(short *)&source[1].m_num_elements;
    if (GameSetup_gData.commMode != 1) {
        pGVar2 = LocateCreateGroupType__15SerializedGroupiP9SimpleMemi(chunkGroup,3,mem,0);
        this->objInstanceBuf = pGVar2;
        pGVar2 = LocateCreateGroupType__15SerializedGroupiP9SimpleMemi(chunkGroup,0xb,mem,0);
        this->simObjBuf = pGVar2;
        goto LAB_8008b734;
    }
    source = LocateGroupType__15SerializedGroupii(chunkGroup,0xb,0);
    source_00 = LocateGroupType__15SerializedGroupii(chunkGroup,3,0);
    if (source == (SerializedGroup *)0x0) {
        LAB_8008b630:
        this->simObjBuf = (Group *)0x0;
    }
    else {
        iVar7 = 0;
        pSVar4 = source + 1;
        while (iVar7 < source->m_num_elements) {
            if (*(char *)((int)&pSVar4[1].m_type + 2) != '\x7f') {
                uVar3 = (uint)*(byte *)((int)&pSVar4[1].m_type + 2);
                pSVar5 = source_00 + 1;
                if (source_00->m_num_elements <= (int)uVar3) break;
                while (uVar3 = uVar3 - 1, uVar3 != 0xffffffff) {
                    pSVar5 = (SerializedGroup *)((int)&pSVar5->m_type + (int)*(short *)&pSVar5->m_type);
                }
                if (((pSVar5 == (SerializedGroup *)0x0) || (*(char *)((int)&pSVar5->m_type + 2) != '\x05'))
                    || (*(char *)((int)&pSVar5[2].m_type + 3) != '\0')) break;
            }
            pSVar4 = (SerializedGroup *)&pSVar4[1].m_length;
            iVar7 = iVar7 + 1;
        }
        if (iVar7 == 0) goto LAB_8008b630;
        pGVar2 = CreateLiteGroupDataSize__15SerializedGroupP15SerializedGroupP9SimpleMemi
                (chunkGroup,source,mem,iVar7 * 0x14);
        this->simObjBuf = pGVar2;
        pGVar2->m_num_elements = iVar7;
    }
    iVar7 = 0;
    if (source_00 == (SerializedGroup *)0x0) {
        this->objInstanceBuf = (Group *)0x0;
    }
    else {
        dataSize = 0;
        pSVar4 = source_00 + 1;
        while (iVar7 < source_00->m_num_elements) {
            cVar1 = *(char *)((int)&pSVar4->m_type + 2);
            if (cVar1 == '\x05') {
                if (*(char *)((int)&pSVar4[2].m_type + 3) != '\0') break;
            }
            else {
                if (cVar1 != '\x01') break;
            }
            iVar7 = iVar7 + 1;
            dataSize = dataSize + (int)*(short *)&pSVar4->m_type;
            pSVar4 = (SerializedGroup *)((int)&pSVar4->m_type + (int)*(short *)&pSVar4->m_type);
        }
        pGVar2 = CreateLiteGroupDataSize__15SerializedGroupP15SerializedGroupP9SimpleMemi
                (chunkGroup,source_00,mem,dataSize);
        this->objInstanceBuf = pGVar2;
        pGVar2->m_num_elements = iVar7;
    }
    if ((this->simObjBuf != (Group *)0x0) && (this->objInstanceBuf != (Group *)0x0)) {
        dataSize = this->simObjBuf->m_num_elements;
        iVar7 = (int)source + dataSize * 0x14 + 0x10;
        while( true ) {
            iVar6 = iVar7;
            dataSize = dataSize + -1;
            if (dataSize == -1) break;
            iVar7 = iVar6 + -0x14;
            if (this->objInstanceBuf->m_num_elements <= (int)(uint)*(byte *)(iVar6 + -2)) {
                *(undefined *)(iVar6 + -2) = 0x7f;
                iVar7 = iVar6 + -0x14;
            }
        }
    }
    LAB_8008b734:
    pGVar2 = LocateCreateGroupType__15SerializedGroupiP9SimpleMemi(chunkGroup,0x15,mem,0);
    this->objSpecialInstanceBuf = pGVar2;
    pGVar2 = LocateCreateGroupType__15SerializedGroupiP9SimpleMemi(chunkGroup,10,mem,0);
    this->sfxBuf = pGVar2;
    pGVar2 = LocateCreateGroupType__15SerializedGroupiP9SimpleMemi(chunkGroup,5,mem,0);
    this->simQuadBuf = pGVar2;
    pGVar2 = LocateCreateGroupType__15SerializedGroupiP9SimpleMemi(chunkGroup,6,mem,0);
    this->simSliceBuf = pGVar2;
    pGVar2 = LocateCreateGroupType__15SerializedGroupiP9SimpleMemi(chunkGroup,9,mem,0);
    this->lineBuf = pGVar2;
    source = LocateGroupType__15SerializedGroupii(chunkGroup,0x17,0);
    source_00 = LocateGroupType__15SerializedGroupii(source,0x1b,0);
    this->quadCounts[0] = *(unsigned char *)&source_00[1].m_num_elements;
    this->quadCounts[1] = *(unsigned char *)((int)&source_00[1].m_num_elements + 2);
    this->quadCounts[2] = *(unsigned char *)&source_00[2].m_type;
    this->quadCounts[3] = *(unsigned char *)((int)&source_00[2].m_type + 2);
    this->quadCounts[4] = *(unsigned char *)&source_00[2].m_length;
    this->quadCounts[5] = *(unsigned char *)((int)&source_00[2].m_length + 2);
    pGVar2 = LocateCreateGroupType__15SerializedGroupiP9SimpleMemi(source,0x19,mem,0);
    *(Group **)this->renderQuads = pGVar2 + 1;
    pGVar2 = (Group *)((int)(pGVar2 + 1) + (int)*(short *)&source_00[1].m_num_elements * 6);
    *(Group **)(this->renderQuads + 1) = pGVar2;
    pGVar2 = (Group *)((int)pGVar2 + (int)*(short *)((int)&source_00[1].m_num_elements + 2) * 6);
    *(Group **)(this->renderQuads + 2) = pGVar2;
    *(Group **)(this->renderQuads + 3) =
            (Group *)((int)pGVar2 + (int)*(short *)&source_00[2].m_length * 6);
    pGVar2 = LocateCreateGroupType__15SerializedGroupiP9SimpleMemi(source,0x1a,mem,0);
    this->stripBuf = pGVar2;
    pGVar2 = LocateCreateGroupType__15SerializedGroupiP9SimpleMemi(source,0x25,mem,0);
    this->lorezstripBuf = pGVar2;
    pGVar2 = LocateCreateGroupType__15SerializedGroupiP9SimpleMemi(source,0x18,mem,0);
    this->vertexBuf = pGVar2;
    pGVar2 = LocateCreateGroupType__15SerializedGroupiP9SimpleMemi(source,0x27,mem,0);
    this->objVertexBuf = pGVar2;
    pGVar2 = LocateCreateGroupType__15SerializedGroupiP9SimpleMemi(source,0x28,mem,0);
    this->objQuadBuf = pGVar2;
    pGVar2 = LocateCreateGroupType__15SerializedGroupiP9SimpleMemi(source,0x29,mem,0);
    this->objQuadInstanceBuf = pGVar2;
    return;
}*/
