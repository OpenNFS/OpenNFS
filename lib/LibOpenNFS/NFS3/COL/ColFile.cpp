#include "ColFile.h"

using namespace LibOpenNFS::NFS3;

bool ColFile::Load(const std::string &colPath, ColFile &colFile) {
    //LOG(INFO) << "Loading COL File located at " << colPath;
    std::ifstream col(colPath, std::ios::in | std::ios::binary);

    bool loadStatus = colFile._SerializeIn(col);
    col.close();

    return loadStatus;
}

void ColFile::Save(const std::string &colPath, ColFile &colFile) {
    //LOG(INFO) << "Saving COL File to " << colPath;
    std::ofstream col(colPath, std::ios::out | std::ios::binary);
    colFile._SerializeOut(col);
}

bool ColFile::_SerializeIn(std::ifstream &ifstream) {
    SAFE_READ(ifstream, &header, sizeof(char) * 4);
    SAFE_READ(ifstream, &version, sizeof(uint32_t));
    SAFE_READ(ifstream, &fileLength, sizeof(uint32_t));
    SAFE_READ(ifstream, &nBlocks, sizeof(uint32_t));

    if ((memcmp(header, "COLL", sizeof(char)) != 0) || (version != 11) || ((nBlocks != 2) && (nBlocks != 4) && (nBlocks != 5))) {
        //LOG(WARNING) << "Invalid COL file";
        return false;
    }

    SAFE_READ(ifstream, xbTable, sizeof(uint32_t) * nBlocks);

    // texture XB
    SAFE_READ(ifstream, &textureHead, sizeof(ExtraBlockHeader));
    if (textureHead.xbid != XBID_TEXTUREINFO) {
        return false;
    }
    texture.resize(textureHead.nrec);
    SAFE_READ(ifstream, texture.data(), sizeof(ColTextureInfo) * textureHead.nrec);

    // struct3D XB
    if (nBlocks >= 4) {
        SAFE_READ(ifstream, &struct3DHead, sizeof(ExtraBlockHeader));
        if (struct3DHead.xbid != XBID_STRUCT3D) {
            return false;
        }
        struct3D.resize(struct3DHead.nrec);
        for (uint32_t colRec_Idx = 0; colRec_Idx < struct3DHead.nrec; colRec_Idx++) {
            SAFE_READ(ifstream, &struct3D[colRec_Idx].size, sizeof(uint32_t));
            SAFE_READ(ifstream, &struct3D[colRec_Idx].nVert, sizeof(uint16_t));
            SAFE_READ(ifstream, &struct3D[colRec_Idx].nPoly, sizeof(uint16_t));

            int32_t delta = (8 + sizeof(ColVertex) * struct3D[colRec_Idx].nVert + sizeof(ColPolygon) * struct3D[colRec_Idx].nPoly) % 4;
            delta         = (4 - delta) % 4;

            // Check the size matches up with the expected size of the contents
            if (struct3D[colRec_Idx].size != 8 + sizeof(ColVertex) * struct3D[colRec_Idx].nVert + sizeof(ColPolygon) * struct3D[colRec_Idx].nPoly + delta) {
                return false;
            }

            // Grab the vertices
            struct3D[colRec_Idx].vertex.resize(struct3D[colRec_Idx].nVert);
            SAFE_READ(ifstream, struct3D[colRec_Idx].vertex.data(), sizeof(ColVertex) * struct3D[colRec_Idx].nVert);

            // And Polygons
            struct3D[colRec_Idx].polygon.resize(struct3D[colRec_Idx].nPoly);
            SAFE_READ(ifstream, struct3D[colRec_Idx].polygon.data(), sizeof(ColPolygon) * struct3D[colRec_Idx].nPoly);

            // Consume the delta, to eat alignment bytes
            int dummy;
            if (delta > 0) {
                SAFE_READ(ifstream, &dummy, delta);
            }
        }

        // TODO: Share this code between both XOBJ parse runs
        // object XB
        SAFE_READ(ifstream, &objectHead, sizeof(ExtraBlockHeader));
        if ((objectHead.xbid != XBID_OBJECT) && (objectHead.xbid != XBID_OBJECT2)) {
            return false;
        }
        object.resize(objectHead.nrec);
        for (uint32_t xobjIdx = 0; xobjIdx < objectHead.nrec; xobjIdx++) {
            SAFE_READ(ifstream, &object[xobjIdx].size, sizeof(uint16_t));
            SAFE_READ(ifstream, &object[xobjIdx].type, sizeof(uint8_t));
            SAFE_READ(ifstream, &object[xobjIdx].struct3D, sizeof(uint8_t));

            if (object[xobjIdx].type == 1) {
                if (object[xobjIdx].size != 16) {
                    return false;
                }
                SAFE_READ(ifstream, &object[xobjIdx].ptRef, sizeof(glm::ivec3));
            } else if (object[xobjIdx].type == 3) {
                SAFE_READ(ifstream, &object[xobjIdx].animLength, sizeof(uint16_t));
                SAFE_READ(ifstream, &object[xobjIdx].unknown, sizeof(uint16_t));
                if (object[xobjIdx].size != 8 + 20 * object[xobjIdx].animLength) {
                    return false;
                }

                object[xobjIdx].animData.resize(object[xobjIdx].animLength);
                SAFE_READ(ifstream, object[xobjIdx].animData.data(), sizeof(AnimData) * object[xobjIdx].animLength);
                // Make a ref point from first anim position
                object[xobjIdx].ptRef = Utils::FixedToFloat(object[xobjIdx].animData[0].pt);
            } else {
                // Unknown object type
                return false;
            }
        }
    }

    // object2 XB
    if (nBlocks == 5) {
        SAFE_READ(ifstream, &object2Head, 8);
        if ((object2Head.xbid != XBID_OBJECT) && (object2Head.xbid != XBID_OBJECT2)) {
            return false;
        }
        object2.resize(object2Head.nrec);
        for (uint32_t xobjIdx = 0; xobjIdx < object2Head.nrec; xobjIdx++) {
            SAFE_READ(ifstream, &object2[xobjIdx].size, sizeof(uint16_t));
            SAFE_READ(ifstream, &object2[xobjIdx].type, sizeof(uint8_t));
            SAFE_READ(ifstream, &object2[xobjIdx].struct3D, sizeof(uint8_t));

            if (object2[xobjIdx].type == 1) {
                if (object2[xobjIdx].size != 16) {
                    return false;
                }
                SAFE_READ(ifstream, &object2[xobjIdx].ptRef, sizeof(glm::ivec3));
            } else if (object2[xobjIdx].type == 3) {
                SAFE_READ(ifstream, &object2[xobjIdx].animLength, sizeof(uint16_t));
                SAFE_READ(ifstream, &object2[xobjIdx].unknown, sizeof(uint16_t));
                if (object2[xobjIdx].size != 8 + 20 * object2[xobjIdx].animLength) {
                    return false;
                }

                object2[xobjIdx].animData.resize(object2[xobjIdx].animLength);
                SAFE_READ(ifstream, object2[xobjIdx].animData.data(), sizeof(AnimData) * object2[xobjIdx].animLength);
                // Make a ref point from first anim position
                object2[xobjIdx].ptRef = Utils::FixedToFloat(object2[xobjIdx].animData[0].pt);
            } else {
                // Unknown object2 type
                return false;
            }
        }
    }

    // vroad XB
    SAFE_READ(ifstream, &vroadHead, 8);
    if (vroadHead.xbid != XBID_VROAD || (vroadHead.size != 8 + sizeof(ColVRoad) * vroadHead.nrec)) {
        return false;
    }
    vroad.resize(vroadHead.nrec);
    SAFE_READ(ifstream, vroad.data(), sizeof(ColVRoad) * vroadHead.nrec);

    return true;
}

void ColFile::_SerializeOut(std::ofstream &ofstream) {
    ASSERT(false, "COL output serialization is not currently implemented");
}
