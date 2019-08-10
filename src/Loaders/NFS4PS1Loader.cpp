//
// Created by Amrik on 09/08/2019.
//

#include <cstring>
#include <fstream>

#include "NFS4PS1Loader.h"
#include "../Util/Logger.h"

/*Transformer_zScene * R3DCar_ReadInCarData(char *filename, Car_tObj *carObj)
{
    int iVar4;
    int iVar5;
    int iVar7;
    short sVar8;
    int iVar9;
    int iVar10;
    extern char **objectInfo;
    uint32_t uVar14;
    char infilename [16];
    int local_70;
    int local_6c;
    int local_68;
    uint16_t local_60;
    uint16_t local_5e;
    short local_5c;
    int datasize;
    short local_40;
    int *local_38;
    uint16_t *local_34;
    int local_30;

    strcpy(infilename,filename);
    strcat(infilename,".geo");
    iVar10 = 0x24c;

    local_38 = &local_70;
    local_34 = &local_60;
    local_30 = 0x7e07e07f;
    SerializedGroup *geoVivEntry = locatebig(0,infilename);
    // No VIV entry gets unpacked, the file just gets located and datasize returns start offset in VIV?
    locatebigentry(0,infilename,0,0,&datasize);
    Transformer_zScene *scene = (Transformer_zScene *)reservememadr(infilename,datasize,0);
    Transformer_zScene *sceneCopy = scene;
    // Move GEO block to Transformer_zScene, size of GEO
    blockmove(geoVivEntry, scene, datasize);
    // This is obviously a special number
    int palCopyNum = (int)(carObj->render).palCopyNum[0xd];
    // LUT of Environment map info
    int local_4c = (&R3DCar_EnvMapInfo)[palCopyNum * 4];
    int local_48 = (&DAT_80116dc0)[palCopyNum * 4];

    // These probably don't matter
    *//**(Cars_tRenderInfo *)(carObj->render).signalLight = (&DAT_80116dc4)[palCopyNum * 4] << 7;
    objectInfo = &R3DCar_ObjectInfo;
    *(Cars_tRenderInfo *)&(carObj->render).currentCarType = (&DAT_80116dc8)[palCopyNum * 4] << 7;*//*

    int partIdx = 0;
    while (partIdx < 0x39) {
        Transformer_zObj *Nobj = (Transformer_zObj *)((int)scene->obj + iVar10);
        // 28?
        iVar10 = iVar10 + 0x1c;
        sceneCopy->obj[partIdx] = Nobj;
        // Apply some part specific translation offsets
        if (partIdx == 0x27) {
            (Nobj->translation).x = (Nobj->translation).x + -0x7ae;
        }
        if (partIdx == 0x28) {
            (Nobj->translation).x = (Nobj->translation).x + 0x7ae;
        }
        uint16_t vertNum = Nobj->numVertex;
        if (vertNum != 0) {
            Nobj->vertex = (COORD16 *)((int)scene->obj + iVar10);
            iVar10 = iVar10 + (uint32_t)vertNum * 6;
            if ((vertNum & 1) != 0) {
                iVar10 = iVar10 + 2;
            }
            if (((vertNum != 0) && ((objectInfo[1] & 1) != 0)) && (palCopyNum < 0x1c)) {
                Nobj->Nvertex = (COORD16 *)((int)scene->obj + iVar10);
                iVar10 = iVar10 + (uint32_t)Nobj->numVertex * 6;
                if ((Nobj->numVertex & 1) != 0) {
                    iVar10 = iVar10 + 2;
                }
                int vertIdx = 0;
                local_40 = (short)((uint32_t)(Nobj->translation).x >> 8);
                iVar4 = (Nobj->translation).y;
                iVar7 = (Nobj->translation).z;
                iVar9 = 0;
                while (vertIdx < (int)(uint32_t)Nobj->numVertex) {
                    local_70 = (int)*(short *)((int)&Nobj->vertex->x + iVar9) + (int)local_40;
                    local_6c = (int)*(short *)((int)&Nobj->vertex->y + iVar9) + (int)(short)((uint32_t)iVar4 >> 8);
                    local_68 = (int)*(short *)((int)&Nobj->vertex->z + iVar9) + (int)(short)((uint32_t)iVar7 >> 8) >> 2;
                    VectorNormalS(local_38,local_34);
                    if ((objectInfo[1] & 0x40) != 0) {
                        local_70 = (int)*(short *)((int)&Nobj->Nvertex->x + iVar9) + (int)(short)local_60;
                        local_6c = (int)*(short *)((int)&Nobj->Nvertex->y + iVar9) + (int)(short)local_5e;
                        local_68 = (int)*(short *)((int)&Nobj->Nvertex->z + iVar9) + (int)local_5c;
                        VectorNormalS(local_38,local_34);
                    }
                    iVar5 = (int)((uint32_t)local_60 << 0x10) >> 0x10;
                    sVar8 = (short)(iVar5 / local_4c);

                    ASSERT(local_4c, "Fuck knows but EA says this is bad so");
                    ASSERT((local_4c != -1) && (iVar5 != -0x80000000) , "Fuck knows but EA says this is bad so");
                    ASSERT(local_48, "Fuck knows but EA says this is bad so");
                    ASSERT((local_48 != -1) && ((int)(short)local_5e != -0x80000000) , "Fuck knows but EA says this is bad so");

                    iVar5 = (int)((long long)iVar5 * (long long)local_30 >> 0x25) - ((int)((uint32_t) local_60 << 0x10) >> 0x1f);
                    uVar14 = ((int)(short)local_5e / local_48) * iVar5;
                    local_60 = (uint16_t)iVar5;
                    local_5c = (short)(int)((long long)(int) local_5c * (long long)local_30 >> 0x25) -
                            (local_5c >> 0xf);
                    local_5e = (uint16_t)uVar14;
                    if (((int)(uVar14 * 0x10000) < 0) && ((uVar14 & 0xff) != 0)) {
                        local_5e = local_5e + 0x100;
                    }
                    uVar14 = (uint32_t)local_5e;
                    local_5e = (short)local_5e >> 8;
                    if (sVar8 < -0x3f) {
                        sVar8 = -0x3f;
                    }
                    if (0x3f < sVar8) {
                        sVar8 = 0x3f;
                    }
                    if ((int)(uVar14 << 0x10) >> 0x18 < -0x3f) {
                        local_5e = -0x3f;
                    }
                    if (0x3f < (short)local_5e) {
                        local_5e = 0x3f;
                    }
                    vertIdx = vertIdx + 1;
                    local_5e = sVar8 - local_5e;
                    *(uint16_t *)((int)&Nobj->Nvertex->x + iVar9) = local_60;
                    *(short *)((int)&Nobj->Nvertex->y + iVar9) = local_5e;
                    *(short *)((int)&Nobj->Nvertex->z + iVar9) = local_5c;
                    iVar9 = iVar9 + 6;
                }
            }
        }
        if (Nobj->numFacet != 0) {
            Nobj->facet = (Transformer_zFacet *)((int)scene->obj + iVar10);
            iVar10 = iVar10 + (uint32_t)Nobj->numFacet * 0xc;
        }
        objectInfo = objectInfo + 6;
        partIdx = partIdx + 1;
    }
    return sceneCopy;
}*/

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <memory.h>

void NFS4PS1Loader::LoadCar() {
    struct stat fstat;
    stat("../resources/NFS_4_PS1/ZZDIAB.GEO", &fstat);
    int size = fstat.st_size;
    char *mem = (char *) malloc(size + 1);

    FILE *geo = fopen("../resources/NFS_4_PS1/ZZDIAB.GEO", "rb");
    ASSERT((int) fread(mem, 1, size, geo) == size, "WEWPS");

    Transformer_zScene *scene = (Transformer_zScene *) mem;

    int fileOffset = 0x24c;
    int partIdx = 0;
    while (partIdx < 57) {
        Transformer_zObj *Nobj = (Transformer_zObj *) ((int) scene->obj + fileOffset);
        fileOffset += 0x1c;
        scene->obj[partIdx] = Nobj;
        // Apply some part specific translation offsets
        if (partIdx == 0x27) {
            (Nobj->translation).x = (Nobj->translation).x + -0x7ae;
        }
        if (partIdx == 0x28) {
            (Nobj->translation).x = (Nobj->translation).x + 0x7ae;
        }
        uint16_t vertNum = Nobj->numVertex;

        std::ofstream obj;
        std::stringstream objPath;
        objPath << "./assets/test.obj";
        obj.open(objPath.str());

        // Print Part name
        obj << "o " << "Part" << partIdx << std::endl;

        if (vertNum != 0) {
            Nobj->vertex = (COORD16 *) ((int) scene->obj + fileOffset);
            fileOffset = fileOffset + (uint32_t) vertNum * 6;
            // Alignment
            if ((vertNum & 1) != 0) {
                fileOffset += 2;
            }
            Nobj->Nvertex = (COORD16 *) ((int) scene->obj + fileOffset);
            fileOffset = fileOffset + (uint32_t) Nobj->numVertex * 6;
            if ((Nobj->numVertex & 1) != 0) {
                fileOffset += 2;
            }
            int vertIdx = 0;
            int translateX = (short) ((uint32_t) (Nobj->translation).x >> 8);
            int translateY = (Nobj->translation).y;
            int translateZ = (Nobj->translation).z;
            int j = 0;
            // Get vertices, calculate normals
            while (vertIdx < (int) (uint32_t) Nobj->numVertex) {
                struct VECTOR vt = {
                        (int) *(short *) ((int) &Nobj->vertex->x + j) + (int) translateX,
                        (int) *(short *) ((int) &Nobj->vertex->y + j) + (int) (short) ((uint32_t) translateY >> 8),
                        (int) (*(short *) ((int) &Nobj->vertex->z + j) + (int) (short) ((uint32_t) translateZ >> 8)) >> 2,
                        0
                };
                obj << "v " <<
                    vt.vx << " "
                    << vt.vy << " "
                    << vt.vz << std::endl;
                // Normal calculation here
                ++vertIdx;
                j += 6;
            }
            obj.close();
        }
        if (Nobj->numFacet != 0) {
            Nobj->facet = (Transformer_zFacet *) ((int) scene->obj + fileOffset);
            fileOffset += (uint32_t) Nobj->numFacet * 0xc;
        }
        ++partIdx;
    }
}
