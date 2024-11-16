#pragma once

#include <iostream>
#include <fstream>
#include <memory>
#include <glm/glm.hpp>

#define ASSERT(condition, message)                                                                                   \
    if (!(condition)) {                                                                                              \
        std::cerr << "Assertion `" #condition "` failed in " << __FILE__ << " line " << __LINE__ << ": " << message; \
        std::terminate();                                                                                            \
    }

#define onfs_check(condition)                                                                              \
    if (!(condition)) {                                                                                    \
        std::cerr << "Check `" #condition "` failed in " << __FILE__ << " line " << __LINE__ << std::endl; \
        return false;                                                                                      \
    }

namespace LibOpenNFS::Utils {
    glm::vec3 FixedToFloat(glm::vec3 fixedPoint);
    bool DecompressCRP(const std::string &compressedCrpPath, const std::string &decompressedCrpPath);
    glm::vec3 CalculateQuadNormal(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p4);
    glm::vec3 CalculateNormal(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3);
    // Easily convert propietary and platform specific Vertices to glm::vec3. NFS2_DATA::PC::GEO::BLOCK_3D, NFS2_DATA::PS1::GEO::BLOCK_3D,
    // NFS3_4_DATA::FLOATPT etc.
    template <class NFSVertexStruct>
    glm::vec3 PointToVec(NFSVertexStruct block_3d) {
        return glm::vec3(block_3d.x, block_3d.y, block_3d.z);
    }
} // namespace LibOpenNFS::Utils
