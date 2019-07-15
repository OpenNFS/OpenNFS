#include "Lib.h"

namespace CrpLib {
    ICrpData* AllocateDataEntry(ENTRY_ID id) {
        ICrpData* data;
        switch (id) {
            case ID_NAME:
                data = new CRawData();
                break;
            case ID_VERTEX:
            case ID_NORMAL:
            case ID_CULL:
                data = new CVector4();
                break;
            case ID_UV:
                data = new CVector2();
                break;
            case ID_EFFECT:
                data = new CEffect();
                break;
            case ID_TRANSFORM:
                data = new CMatrix();
                break;
            case ID_PART:
                data = new CPart();
                break;
            case ID_BASE:
                data = new CBase();
                break;
            case ID_BPLANES:
                data = new CBPlanes();
                break;
            case ID_MATERIAL:
                data = new CMaterial();
                break;
            default:
                data = new CRawData();
                break;
        }
        return data;
    }

}