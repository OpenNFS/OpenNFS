#include "PolyBlock.h"


PolyBlock::PolyBlock(std::ifstream &frd, uint32_t nTrackBlockPolys) : m_nTrackBlockPolys(nTrackBlockPolys), obj {}
{
    this->obj.reserve(NUM_POLYOBJ_CHUNKS);
    ASSERT(this->_SerializeIn(frd), "Failed to serialize PolyBlock from file stream");
}

bool PolyBlock::_SerializeIn(std::ifstream &frd)
{
    for (uint32_t polyBlockIdx = 0; polyBlockIdx < NUM_POLYGON_BLOCKS; polyBlockIdx++)
    {
        SAFE_READ(frd, &(sz[polyBlockIdx]), sizeof(uint32_t));
        if (sz[polyBlockIdx] != 0)
        {
            SAFE_READ(frd, &(szdup[polyBlockIdx]), 0x4);
            if (szdup[polyBlockIdx] != sz[polyBlockIdx])
            {
                return false;
            }
            SAFE_READ(frd, &poly[polyBlockIdx], sizeof(PolygonData) * sz[polyBlockIdx]);
        }
    }

    // Sanity check
    if (sz[4] != m_nTrackBlockPolys)
    {
        return false;
    }

    for(uint8_t objIdx = 0; objIdx < NUM_POLYOBJ_CHUNKS; ++objIdx)
    {
        auto o = std::make_unique<ObjectPolyBlock>();
        SAFE_READ(frd, &o->n1, sizeof(uint32_t));

        if (o->n1 > 0)
        {
            SAFE_READ(frd, &o->n2, sizeof(uint32_t));
            o->types.resize(o->n2);
            o->numpoly.resize(o->n2);
            o->poly.resize(o->n2);

            uint32_t polygonCount = 0;
            o->nobj = 0;
            for (uint32_t k = 0; k < o->n2; ++k)
            {
                SAFE_READ(frd, &o->types[k], sizeof(uint32_t));
                if (o->types[k] == 1)
                {
                    SAFE_READ(frd, o->numpoly.data() + o->nobj, sizeof(uint32_t));

                    o->poly[o->nobj] = std::vector<PolygonData>(o->numpoly[o->nobj]);
                    SAFE_READ(frd, o->poly[o->nobj].data(), sizeof(PolygonData) * o->numpoly[o->nobj]);

                    polygonCount += o->numpoly[o->nobj];
                    ++o->nobj;
                }
            }
            // n1 == total nb polygons
            if (polygonCount != o->n1)
            {
                return false;
            }
        }
        // TODO: For some reason this is a segfault. I'd rather vector containers than raw uint32_t *, solution needed. :(
        //obj.push_back(std::move(o));
    }

    return true;
}

void PolyBlock::SerializeOut(std::ofstream &frd)
{
    ASSERT(false, "PolyBlock serialization to file stream is not implemented");
}

