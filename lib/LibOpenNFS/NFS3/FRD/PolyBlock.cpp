#include "PolyBlock.h"

using namespace LibOpenNFS::NFS3;

PolyBlock::PolyBlock(std::ifstream &frd, uint32_t nTrackBlockPolys) : m_nTrackBlockPolys(nTrackBlockPolys), obj{} {
    ASSERT(this->_SerializeIn(frd), "Failed to serialize PolyBlock from file stream");
}

bool PolyBlock::_SerializeIn(std::ifstream &ifstream) {
    for (uint32_t polyBlockIdx = 0; polyBlockIdx < NUM_POLYGON_BLOCKS; polyBlockIdx++) {
        onfs_check(safe_read(ifstream, sz[polyBlockIdx]));
        if (sz[polyBlockIdx] != 0) {
            onfs_check(safe_read(ifstream, szdup[polyBlockIdx]));
            if (szdup[polyBlockIdx] != sz[polyBlockIdx]) {
                return false;
            }
            poly[polyBlockIdx].resize(sz[polyBlockIdx]);
            onfs_check(safe_read(ifstream, poly[polyBlockIdx]));
        }
    }

    // Sanity check
    if (sz[4] != m_nTrackBlockPolys) {
        return false;
    }

    for (auto &o : obj) {
        onfs_check(safe_read(ifstream, o.n1));
        if (o.n1 > 0) {
            onfs_check(safe_read(ifstream, o.n2));

            o.types.resize(o.n2);
            o.numpoly.resize(o.n2);
            o.poly.resize(o.n2);

            uint32_t polygonCount = 0;
            o.nobj                = 0;

            for (uint32_t k = 0; k < o.n2; ++k) {
                onfs_check(safe_read(ifstream, o.types[k]));

                if (o.types[k] == 1) {
                    onfs_check(safe_read(ifstream, o.numpoly[o.nobj]));

                    o.poly[o.nobj].resize(o.numpoly[o.nobj]);
                    onfs_check(safe_read(ifstream, o.poly[o.nobj]));

                    polygonCount += o.numpoly[o.nobj];
                    ++o.nobj;
                }
            }
            // n1 == total nb polygons
            if (polygonCount != o.n1) {
                return false;
            }
        }
    }

    return true;
}

void PolyBlock::_SerializeOut(std::ofstream &ofstream) {
    for (uint32_t polyBlockIdx = 0; polyBlockIdx < NUM_POLYGON_BLOCKS; polyBlockIdx++) {
        ofstream.write((char *) &sz[polyBlockIdx], sizeof(uint32_t));
        if (sz[polyBlockIdx] != 0) {
            ofstream.write((char *) &szdup[polyBlockIdx], sizeof(uint32_t));
            ofstream.write((char *) poly[polyBlockIdx].data(), sizeof(PolygonData) * sz[polyBlockIdx]);
        }
    }

    for (auto &o : obj) {
        ofstream.write((char *) &o.n1, sizeof(uint32_t));
        if (o.n1 > 0) {
            ofstream.write((char *) &o.n2, sizeof(uint32_t));
            o.nobj = 0;
            for (uint32_t k = 0; k < o.n2; ++k) {
                ofstream.write((char *) &o.types[k], sizeof(uint32_t));
                if (o.types[k] == 1) {
                    ofstream.write((char *) &o.numpoly[o.nobj], sizeof(uint32_t));
                    ofstream.write((char *) o.poly[o.nobj].data(), sizeof(PolygonData) * o.numpoly[o.nobj]);
                    ++o.nobj;
                }
            }
        }
    }
}
