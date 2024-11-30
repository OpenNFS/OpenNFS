#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <vector>
#include <filesystem>

#include <LinearMath/btVector3.h>
#include <LinearMath/btQuaternion.h>
#include <LinearMath/btMatrix3x3.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <BulletCollision/CollisionShapes/btBoxShape.h>

#include "Logger.h"

namespace OpenNFS {
    struct DimensionData {
        glm::vec3 minVertex;
        glm::vec3 maxVertex;
    };

    namespace Utils {
        glm::vec3 bulletToGlm(const btVector3 &v);
        btVector3 glmToBullet(const glm::vec3 &v);
        glm::quat bulletToGlm(const btQuaternion &q);
        btQuaternion glmToBullet(const glm::quat &q);
        btMatrix3x3 glmToBullet(const glm::mat3 &m);
        btTransform glmToBullet(const glm::mat4 &m);
        glm::mat4 bulletToGlm(const btTransform &t);

        float RandomFloat(float min, float max);
        DimensionData GenDimensions(const std::vector<glm::vec3> &vertices);
        btTransform MakeTransform(glm::vec3 position, glm::quat orientation);

        std::vector<NfsAssetList> PopulateAssets();
        static bool FilePathSortByDepthReverse(const std::filesystem::path &a, const std::filesystem::path &b);
        void RenameAssetsToLowercase();
    } // namespace Utils

    // Avoid requiring C++23 just for this. If we upgrade, nerf this.
    [[noreturn]] inline void unreachable() {
        // Uses compiler specific extensions if possible.
        // Even if no extension is used, undefined behavior is still raised by
        // an empty function body and the noreturn attribute.
#if defined(_MSC_VER) && !defined(__clang__) // MSVC
        __assume(false);
#else // GCC, Clang
        __builtin_unreachable();
#endif
    }
} // namespace OpenNFS
