#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <vector>
#include <random>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <cstdio>
#include <sstream>

#include <LinearMath/btVector3.h>
#include <LinearMath/btQuaternion.h>
#include <LinearMath/btMatrix3x3.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <BulletCollision/CollisionShapes/btBoxShape.h>

#include "Logger.h"

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
    DimensionData GenDimensions(std::vector<glm::vec3> vertices);
    btTransform MakeTransform(glm::vec3 position, glm::quat orientation);

    std::vector<NfsAssetList> PopulateAssets();
    static bool FilePathSortByDepthReverse(std::filesystem::path a, std::filesystem::path b);
    void RenameAssetsToLowercase();
} // namespace Utils
