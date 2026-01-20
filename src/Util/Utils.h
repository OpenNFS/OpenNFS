#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <filesystem>
#include <vector>

#include <BulletCollision/CollisionShapes/btBoxShape.h>
#include <LinearMath/btMatrix3x3.h>
#include <LinearMath/btQuaternion.h>
#include <LinearMath/btVector3.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Logger.h"

namespace OpenNFS {
    struct DimensionData {
        glm::vec3 minVertex;
        glm::vec3 maxVertex;
    };

    namespace Utils {
        glm::mat4 rotAroundPoint(float rad, glm::vec3 const &point, glm::vec3 const &axis);
        glm::vec3 bulletToGlm(btVector3 const &v);
        btVector3 glmToBullet(glm::vec3 const &v);
        glm::quat bulletToGlm(btQuaternion const &q);
        btQuaternion glmToBullet(glm::quat const &q);
        btMatrix3x3 glmToBullet(glm::mat3 const &m);
        btTransform glmToBullet(glm::mat4 const &m);
        glm::mat4 bulletToGlm(btTransform const &t);

        float RandomFloat(float min, float max);
        DimensionData GenDimensions(std::vector<glm::vec3> const &vertices);
        btTransform MakeTransform(glm::vec3 position, glm::quat orientation);

        std::vector<NfsAssetList> PopulateAssets();
        static bool FilePathSortByDepthReverse(std::filesystem::path const &a, std::filesystem::path const &b);
        void RenameAssetsToLowercase();

        template <auto... Values> struct OneOf {
            template <typename T> constexpr bool operator==(T const &value) const {
                return ((value == Values) || ...);
            }

            template <typename T> constexpr friend bool operator==(T const &value, OneOf const &) {
                return ((value == Values) || ...);
            }
        };
    } // namespace Utils

    // Avoid requiring C++23 just for this. If we upgrade, nerf this.
    [[noreturn]] inline void unreachable() {
        // Uses compiler specific extensions if possible.
        // Even if no extension is used, undefined behaviour is still raised by
        // an empty function body and the noreturn attribute.
#if defined(_MSC_VER) && !defined(__clang__) // MSVC
        __assume(false);
#else // GCC, Clang
        __builtin_unreachable();
#endif
    }
} // namespace OpenNFS
