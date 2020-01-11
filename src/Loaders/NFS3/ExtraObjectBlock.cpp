#include "ExtraObjectBlock.h"

ExtraObjectBlock::ExtraObjectBlock(std::ifstream &frd)
{
    ASSERT(this->_SerializeIn(frd), "Failed to serialize ExtraObjectBlock from file stream");
}

bool ExtraObjectBlock::_SerializeIn(std::ifstream &frd)
{
    SAFE_READ(frd, &(nobj), sizeof(uint32_t));
    obj.reserve(nobj);
    
    for (uint32_t xobjIdx = 0; xobjIdx < nobj; ++xobjIdx)
    {
        ExtraObjectData x;

        SAFE_READ(frd, &x.crosstype, sizeof(uint32_t));
        SAFE_READ(frd, &x.crossno, sizeof(uint32_t));
        SAFE_READ(frd, &x.unknown, sizeof(uint32_t));

        if (x.crosstype == 4)
        {
            // Basic objects
            SAFE_READ(frd, &x.ptRef, sizeof(glm::vec3));
            SAFE_READ(frd, &x.AnimMemory, sizeof(uint32_t));
        }
        else if (x.crosstype == 3)
        {
            // Animated objects
            SAFE_READ(frd, &x.unknown3, sizeof(uint16_t) * 9);
            SAFE_READ(frd, &x.type3, sizeof(uint8_t));
            SAFE_READ(frd, &x.objno, sizeof(uint8_t));
            SAFE_READ(frd, &x.nAnimLength, sizeof(uint16_t));
            SAFE_READ(frd, &x.AnimDelay, sizeof(uint16_t));

            // Sanity Check
            if (x.type3 != 3)
            {
                return false;
            }

            x.animData.reserve(x.nAnimLength);
            SAFE_READ(frd, x.animData.data(), sizeof(AnimData) * x.nAnimLength);
            // make a ref point from first anim position
            x.ptRef.x = (float) (x.animData[0].pt.x / 65536.0);
            x.ptRef.z = (float) (x.animData[0].pt.z / 65536.0);
            x.ptRef.y = (float) (x.animData[0].pt.y / 65536.0);
        }
        else return false; // unknown object type

        // Get number of vertices
        SAFE_READ(frd, &(x.nVertices), 4);

        // Get vertices
        x.vert.reserve(x.nVertices);
        SAFE_READ(frd, x.vert.data(), sizeof(glm::vec3) * x.nVertices);

        // Per vertex shading data (RGBA)
        x.vertShading.reserve(x.nVertices);
        SAFE_READ(frd, x.vertShading.data(), sizeof(uint32_t) * x.nVertices);

        // Get number of polygons
        SAFE_READ(frd, &(x.nPolygons), sizeof(uint32_t));

        // Grab the polygons
        x.polyData.reserve(x.nPolygons);
        SAFE_READ(frd, x.polyData.data(), sizeof(PolygonData) * x.nPolygons);

        obj.push_back(x);
    }

    return true;
}

void ExtraObjectBlock::SerializeOut(std::ofstream &frd)
{
    ASSERT(false, "ExtraObjectBlock serialization to file stream is not implemented");
}