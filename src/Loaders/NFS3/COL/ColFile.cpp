#include "ColFile.h"


bool ColFile::Load(const std::string &colPath, ColFile &colFile)
{
    LOG(INFO) << "Loading COL File located at " << colPath;
    std::ifstream col(colPath, std::ios::in | std::ios::binary);

    bool loadStatus = colFile._SerializeIn(col);
    col.close();

    return loadStatus;
}

void ColFile::Save(const std::string &colPath, ColFile &colFile)
{
    LOG(INFO) << "Saving COL File to " << colPath;
    std::ofstream col(colPath, std::ios::out | std::ios::binary);
    colFile._SerializeOut(col);
}

bool ColFile::_SerializeIn(std::ifstream &ifstream)
{
    ColObject *o;
    hs_extra = NULL;

    SAFE_READ(ifstream, &header, sizeof(char) * 4);
    SAFE_READ(ifstream, &version, sizeof(uint32_t));
    SAFE_READ(ifstream, &fileLength, sizeof(uint32_t));
    SAFE_READ(ifstream, &nBlocks, sizeof(uint32_t));

    if ((memcmp(header, "COLL", sizeof(char)) != 0) || (version != 11) || ((nBlocks != 2) && (nBlocks != 4) && (nBlocks != 5)))
    {
        LOG(WARNING) << "Invalid COL file";
        return false;
    }

    SAFE_READ(ifstream, xbTable, 4 * nBlocks);

    // texture XB
    SAFE_READ(ifstream, &textureHead, 8);
    if (textureHead.xbid != XBID_TEXTUREINFO)
    {
        return false;
    }

    texture = new ColTextureInfo[textureHead.nrec];
    SAFE_READ(ifstream, texture, 8 * textureHead.nrec);

    // struct3D XB
    if (nBlocks >= 4)
    {
        SAFE_READ(ifstream, &struct3DHead, 8);
        if (struct3DHead.xbid != XBID_STRUCT3D)
        {
            return false;
        }
        ColStruct3D *s = struct3D = new ColStruct3D[struct3DHead.nrec];
        for (uint32_t colRec_Idx = 0; colRec_Idx < struct3DHead.nrec; colRec_Idx++, s++)
        {
            SAFE_READ(ifstream, s, 8);
            int delta = (8 + 16 * s->nVert + 6 * s->nPoly) % 4;
            delta = (4 - delta) % 4;
            if (s->size != 8 + 16 * s->nVert + 6 * s->nPoly + delta)
            { return false; }
            s->vertex = new ColVertex[s->nVert];
            SAFE_READ(ifstream, s->vertex, 16 * s->nVert);
            s->polygon = new ColPolygon[s->nPoly];
            SAFE_READ(ifstream, s->polygon, 6 * s->nPoly);
            int dummy;
            if (delta > 0) SAFE_READ(ifstream, &dummy, delta);
        }

        // object XB
        SAFE_READ(ifstream, &objectHead, 8);
        if ((objectHead.xbid != XBID_OBJECT) && (objectHead.xbid != XBID_OBJECT2))
        {
            return false;
        }
        o = object = new ColObject[objectHead.nrec];

        for (uint32_t colRec_Idx = 0; colRec_Idx < objectHead.nrec; colRec_Idx++, o++)
        {
            SAFE_READ(ifstream, o, 4);
            if (o->type == 1)
            {
                if (o->size != 16)
                { return false; }
                SAFE_READ(ifstream, &(o->ptRef), 12);
            } else if (o->type == 3)
            {
                SAFE_READ(ifstream, &(o->animLength), 4);
                if (o->size != 8 + 20 * o->animLength)
                { return false; }
                o->animData = new AnimData[o->animLength];
                SAFE_READ(ifstream, o->animData, 20 * o->animLength);
                o->ptRef.x = o->animData->pt.x;
                o->ptRef.z = o->animData->pt.z;
                o->ptRef.y = o->animData->pt.y;
            } else
            { return false; } // unknown object type
        }
    }

    // object2 XB
    if (nBlocks == 5)
    {
        SAFE_READ(ifstream, &object2Head, 8);
        if ((object2Head.xbid != XBID_OBJECT) && (object2Head.xbid != XBID_OBJECT2))
        {
            return false;
        }
        o = object2 = new ColObject[object2Head.nrec];

        for (uint32_t colRec_Idx = 0; colRec_Idx < object2Head.nrec; colRec_Idx++, o++)
        {
            SAFE_READ(ifstream, o, 4);
            if (o->type == 1)
            {
                if (o->size != 16)
                { return false; }
                SAFE_READ(ifstream, &(o->ptRef), 12);
            } else if (o->type == 3)
            {
                SAFE_READ(ifstream, &(o->animLength), 4);
                if (o->size != 8 + 20 * o->animLength)
                {
                    return false;
                }
                o->animData = new AnimData[o->animLength];
                SAFE_READ(ifstream, o->animData, 20 * o->animLength);
                o->ptRef.x = o->animData->pt.x;
                o->ptRef.z = o->animData->pt.z;
                o->ptRef.y = o->animData->pt.y;
            } else
            {
                // unknown object type
                return false;
            }
        }
    }

    // vroad XB
    SAFE_READ(ifstream, &vroadHead, 8);
    if (vroadHead.xbid != XBID_VROAD)
    {
        return false;
    }
    if (vroadHead.size != 8 + 36 * vroadHead.nrec)
    {
        return false;
    }
    vroad = new ColVRoad[vroadHead.nrec];
    SAFE_READ(ifstream, vroad, 36 * vroadHead.nrec);

    uint32_t pad;
    return ifstream.read((char *) &pad, 4).gcount() == 0; // we ought to be at EOF now
}

void ColFile::_SerializeOut(std::ofstream &ofstream)
{
    ASSERT(false, "COL output serialization is not currently implemented");
}
