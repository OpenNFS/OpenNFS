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
        SAFE_READ(ifstream, &unknown, sizeof(uint32_t));
        SAFE_READ(ifstream, &nTriangles, sizeof(uint32_t));
        SAFE_READ(ifstream, &nVertices, sizeof(uint32_t));
        SAFE_READ(ifstream, &nArts, sizeof(uint32_t));
        SAFE_READ(ifstream, &vertTblOffset, sizeof(uint32_t));
        SAFE_READ(ifstream, &normTblOffset, sizeof(uint32_t));
        SAFE_READ(ifstream, &triTblOffset, sizeof(uint32_t));
        SAFE_READ(ifstream, &reserve1Offset, sizeof(uint32_t));
        SAFE_READ(ifstream, &reserve2Offset, sizeof(uint32_t));
        SAFE_READ(ifstream, &reserve3Offset, sizeof(uint32_t));
        SAFE_READ(ifstream, &modelHalfSize, sizeof(glm::vec3));
        SAFE_READ(ifstream, &nDummies, sizeof(uint32_t));
        SAFE_READ(ifstream, &dummyCoords, sizeof(glm::vec3) * 16);
        SAFE_READ(ifstream, &nParts, sizeof(uint32_t));
        SAFE_READ(ifstream, &partCoords, sizeof(glm::vec3) * 64);
        SAFE_READ(ifstream, &partFirstVertIndices, sizeof(uint32_t) * 64);
        SAFE_READ(ifstream, &partNumVertices, sizeof(uint32_t) * 64);
        SAFE_READ(ifstream, &partFirstTriIndices, sizeof(uint32_t) * 64);
        SAFE_READ(ifstream, &partNumTriangles, sizeof(uint32_t) * 64);
        SAFE_READ(ifstream, &nPriColours, sizeof(uint32_t));
        SAFE_READ(ifstream, &primaryColours, sizeof(Colour) * 16);
        SAFE_READ(ifstream, &nSecColours, sizeof(uint32_t));
        SAFE_READ(ifstream, &secondaryColours, sizeof(Colour) * 16);
        SAFE_READ(ifstream, &dummyNames, sizeof(char) * 16 * 64);
        SAFE_READ(ifstream, &partNames, sizeof(char) * 64 * 64);
        SAFE_READ(ifstream, &unknownTable, sizeof(uint32_t) * 64);

        carParts.resize(nParts);

        for (uint32_t partIdx = 0; partIdx < nParts; ++partIdx) {
            carParts[partIdx].vertices.resize(partNumVertices[partIdx]);
            carParts[partIdx].normals.resize(partNumVertices[partIdx]);
            carParts[partIdx].triangles.resize(partNumTriangles[partIdx]);

            ifstream.seekg(0x1F04 + vertTblOffset + (partFirstVertIndices[partIdx] * sizeof(glm::vec3)), std::ios_base::beg);
            SAFE_READ(ifstream, carParts[partIdx].vertices.data(), partNumVertices[partIdx] * sizeof(glm::vec3));

            ifstream.seekg(0x1F04 + normTblOffset + (partFirstVertIndices[partIdx] * sizeof(glm::vec3)), std::ios_base::beg);
            ifstream.read((char *) carParts[partIdx].normals.data(), partNumVertices[partIdx] * sizeof(glm::vec3));

            ifstream.seekg(0x1F04 + triTblOffset + (partFirstTriIndices[partIdx] * sizeof(Triangle)), std::ios_base::beg);
            ifstream.read((char *) carParts[partIdx].triangles.data(), partNumTriangles[partIdx] * sizeof(Triangle));
        }

        return true;
    }

    void FceFile::_SerializeOut(std::ofstream &ofstream) {
        ASSERT(false, "FCE output serialization is not currently implemented");
    }
} // namespace LibOpenNFS::NFS3
