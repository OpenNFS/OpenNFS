#include "ColFile.h"

#include <cstring>

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
    onfs_check(safe_read(ifstream, header, sizeof(char) * 4));
    onfs_check(safe_read(ifstream, version));
    onfs_check(safe_read(ifstream, fileLength));
    onfs_check(safe_read(ifstream, nBlocks));

    if ((memcmp(header, "COLL", sizeof(char)) != 0) || (version != 11) || ((nBlocks != 2) && (nBlocks != 4) && (nBlocks != 5))) {
        //LOG(WARNING) << "Invalid COL file";
        return false;
    }

    onfs_check(safe_read(ifstream, xbTable, sizeof(uint32_t) * nBlocks));

    // texture XB
    onfs_check(safe_read(ifstream, textureHead));
    if (textureHead.xbid != XBID_TEXTUREINFO) {
        return false;
    }
    texture.resize(textureHead.nrec);
    onfs_check(safe_read(ifstream, texture));

    // struct3D XB
    if (nBlocks >= 4) {
        onfs_check(safe_read(ifstream, struct3DHead));
        if (struct3DHead.xbid != XBID_STRUCT3D) {
            return false;
        }
        struct3D.resize(struct3DHead.nrec);
        for (uint32_t colRec_Idx = 0; colRec_Idx < struct3DHead.nrec; colRec_Idx++) {
            onfs_check(safe_read(ifstream, struct3D[colRec_Idx].size));
            onfs_check(safe_read(ifstream, struct3D[colRec_Idx].nVert));
            onfs_check(safe_read(ifstream, struct3D[colRec_Idx].nPoly));

            int32_t delta = (8 + sizeof(ColVertex) * struct3D[colRec_Idx].nVert + sizeof(ColPolygon) * struct3D[colRec_Idx].nPoly) % 4;
            delta         = (4 - delta) % 4;

            // Check the size matches up with the expected size of the contents
            if (struct3D[colRec_Idx].size != 8 + sizeof(ColVertex) * struct3D[colRec_Idx].nVert + sizeof(ColPolygon) * struct3D[colRec_Idx].nPoly + delta) {
                return false;
            }

            // Grab the vertices
            struct3D[colRec_Idx].vertex.resize(struct3D[colRec_Idx].nVert);
            onfs_check(safe_read(ifstream, struct3D[colRec_Idx].vertex));

            // And Polygons
            struct3D[colRec_Idx].polygon.resize(struct3D[colRec_Idx].nPoly);
            onfs_check(safe_read(ifstream, struct3D[colRec_Idx].polygon));

            // Consume the delta, to eat alignment bytes
            int dummy;
            if (delta > 0) {
                onfs_check(safe_read(ifstream, dummy, delta));
            }
        }

        // TODO: Share this code between both XOBJ parse runs
        // object XB
        onfs_check(safe_read(ifstream, objectHead));
        if ((objectHead.xbid != XBID_OBJECT) && (objectHead.xbid != XBID_OBJECT2)) {
            return false;
        }
        object.resize(objectHead.nrec);
        for (uint32_t xobjIdx = 0; xobjIdx < objectHead.nrec; xobjIdx++) {
            onfs_check(safe_read(ifstream, object[xobjIdx].size));
            onfs_check(safe_read(ifstream, object[xobjIdx].type, sizeof(uint8_t)));
            onfs_check(safe_read(ifstream, object[xobjIdx].struct3D, sizeof(uint8_t)));

            if (object[xobjIdx].type == 1) {
                if (object[xobjIdx].size != 16) {
                    return false;
                }
                onfs_check(safe_read(ifstream, object[xobjIdx].ptRef, sizeof(glm::ivec3)));
            } else if (object[xobjIdx].type == 3) {
                onfs_check(safe_read(ifstream, object[xobjIdx].animLength, sizeof(uint16_t)));
                onfs_check(safe_read(ifstream, object[xobjIdx].unknown, sizeof(uint16_t)));
                if (object[xobjIdx].size != 8 + 20 * object[xobjIdx].animLength) {
                    return false;
                }

                object[xobjIdx].animData.resize(object[xobjIdx].animLength);
                onfs_check(safe_read(ifstream, object[xobjIdx].animData));
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
        onfs_check(safe_read(ifstream, object2Head, 8));
        if ((object2Head.xbid != XBID_OBJECT) && (object2Head.xbid != XBID_OBJECT2)) {
            return false;
        }
        object2.resize(object2Head.nrec);
        for (uint32_t xobjIdx = 0; xobjIdx < object2Head.nrec; xobjIdx++) {
            onfs_check(safe_read(ifstream, object2[xobjIdx].size, sizeof(uint16_t)));
            onfs_check(safe_read(ifstream, object2[xobjIdx].type, sizeof(uint8_t)));
            onfs_check(safe_read(ifstream, object2[xobjIdx].struct3D, sizeof(uint8_t)));

            if (object2[xobjIdx].type == 1) {
                if (object2[xobjIdx].size != 16) {
                    return false;
                }
                onfs_check(safe_read(ifstream, object2[xobjIdx].ptRef, sizeof(glm::ivec3)));
            } else if (object2[xobjIdx].type == 3) {
                onfs_check(safe_read(ifstream, object2[xobjIdx].animLength, sizeof(uint16_t)));
                onfs_check(safe_read(ifstream, object2[xobjIdx].unknown, sizeof(uint16_t)));
                if (object2[xobjIdx].size != 8 + 20 * object2[xobjIdx].animLength) {
                    return false;
                }

                object2[xobjIdx].animData.resize(object2[xobjIdx].animLength);
                onfs_check(safe_read(ifstream, object2[xobjIdx].animData));
                // Make a ref point from first anim position
                object2[xobjIdx].ptRef = Utils::FixedToFloat(object2[xobjIdx].animData[0].pt);
            } else {
                // Unknown object2 type
                return false;
            }
        }
    }

    // vroad XB
    onfs_check(safe_read(ifstream, vroadHead, 8));
    if (vroadHead.xbid != XBID_VROAD || (vroadHead.size != 8 + sizeof(ColVRoad) * vroadHead.nrec)) {
        return false;
    }
    vroad.resize(vroadHead.nrec);
    onfs_check(safe_read(ifstream, vroad));

    return true;
}

void ColFile::_SerializeOut(std::ofstream &ofstream) {
    ASSERT(false, "COL output serialization is not currently implemented");
}
