#include "FceFile.h"

namespace LibOpenNFS::NFS3 {
    bool FceFile::Load(const std::string &fcePath, FceFile &fceFile) {
        //LOG(INFO) << "Loading FCE File located at " << fcePath;
        std::ifstream fce(fcePath, std::ios::in | std::ios::binary);

        bool loadStatus = fceFile._SerializeIn(fce);
        fce.close();

        return loadStatus;
    }

    void FceFile::Save(const std::string &fcePath, FceFile &fceFile) {
        //LOG(INFO) << "Saving FCE File to " << fcePath;
        std::ofstream fce(fcePath, std::ios::out | std::ios::binary);
        fceFile._SerializeOut(fce);
    }

    bool FceFile::_SerializeIn(std::ifstream &ifstream) {
        onfs_check(safe_read(ifstream, unknown));
        onfs_check(safe_read(ifstream, nTriangles));
        onfs_check(safe_read(ifstream, nVertices));
        onfs_check(safe_read(ifstream, nArts));
        onfs_check(safe_read(ifstream, vertTblOffset));
        onfs_check(safe_read(ifstream, normTblOffset));
        onfs_check(safe_read(ifstream, triTblOffset));
        onfs_check(safe_read(ifstream, reserve1Offset));
        onfs_check(safe_read(ifstream, reserve2Offset));
        onfs_check(safe_read(ifstream, reserve3Offset));
        onfs_check(safe_read(ifstream, modelHalfSize, sizeof(glm::vec3)));
        onfs_check(safe_read(ifstream, nDummies));
        onfs_check(safe_read(ifstream, dummyCoords, sizeof(glm::vec3) * 16));
        onfs_check(safe_read(ifstream, nParts));
        onfs_check(safe_read(ifstream, partCoords, sizeof(glm::vec3) * 64));
        onfs_check(safe_read(ifstream, partFirstVertIndices, sizeof(uint32_t) * 64));
        onfs_check(safe_read(ifstream, partNumVertices, sizeof(uint32_t) * 64));
        onfs_check(safe_read(ifstream, partFirstTriIndices, sizeof(uint32_t) * 64));
        onfs_check(safe_read(ifstream, partNumTriangles, sizeof(uint32_t) * 64));
        onfs_check(safe_read(ifstream, nPriColours, sizeof(uint32_t)));
        onfs_check(safe_read(ifstream, primaryColours, sizeof(Colour) * 16));
        onfs_check(safe_read(ifstream, nSecColours, sizeof(uint32_t)));
        onfs_check(safe_read(ifstream, secondaryColours, sizeof(Colour) * 16));
        onfs_check(safe_read(ifstream, dummyNames, sizeof(char) * 16 * 64));
        onfs_check(safe_read(ifstream, partNames, sizeof(char) * 64 * 64));
        onfs_check(safe_read(ifstream, unknownTable, sizeof(uint32_t) * 64));

        carParts.resize(nParts);

        for (uint32_t partIdx = 0; partIdx < nParts; ++partIdx) {
            carParts[partIdx].vertices.resize(partNumVertices[partIdx]);
            carParts[partIdx].normals.resize(partNumVertices[partIdx]);
            carParts[partIdx].triangles.resize(partNumTriangles[partIdx]);

            ifstream.seekg(0x1F04 + vertTblOffset + (partFirstVertIndices[partIdx] * sizeof(glm::vec3)), std::ios_base::beg);
            onfs_check(safe_read(ifstream, carParts[partIdx].vertices));

            ifstream.seekg(0x1F04 + normTblOffset + (partFirstVertIndices[partIdx] * sizeof(glm::vec3)), std::ios_base::beg);
            onfs_check(safe_read(ifstream, carParts[partIdx].normals));

            ifstream.seekg(0x1F04 + triTblOffset + (partFirstTriIndices[partIdx] * sizeof(Triangle)), std::ios_base::beg);
            onfs_check(safe_read(ifstream, carParts[partIdx].triangles));
        }

        return true;
    }

    void FceFile::_SerializeOut(std::ofstream &ofstream) {
        ASSERT(false, "FCE output serialization is not currently implemented");
    }
} // namespace LibOpenNFS::NFS3
