#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <vector>
#include <random>
#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <cstdio>
#include <sstream>
#include <iomanip>
#include <chrono>

#include <LinearMath/btVector3.h>
#include <LinearMath/btQuaternion.h>
#include <LinearMath/btMatrix3x3.h>
#include <tiny_obj_loader.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <BulletCollision/CollisionShapes/btBoxShape.h>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <GL/glew.h>

#include "Logger.h"
#include "../Scene/CarModel.h"
#include "../Enums.h"

#define SAFE_READ(file, structure, size)  if((file).read((char *) (structure), (size)).gcount() != (size)) return false

struct DimensionData
{
    glm::vec3 minVertex;
    glm::vec3 maxVertex;
};

namespace Utils {
    class Timer {
    public:
        Timer() : m_beg(clock_::now()) {
        }
        void reset() {
            m_beg = clock_::now();
        }

        double elapsed() const {
            return std::chrono::duration_cast<std::chrono::milliseconds>(
                    clock_::now() - m_beg).count();
        }

    private:
        typedef std::chrono::high_resolution_clock clock_;
        typedef std::chrono::duration<double, std::ratio<1> > second_;
        std::chrono::time_point<clock_> m_beg;
    };


    float RandomFloat(float min, float max);

    glm::vec3 bulletToGlm(const btVector3 &v);

    btVector3 glmToBullet(const glm::vec3 &v);

    // btTransform does not contain a full 4x4 matrix, so this transform is lossy.
    // Affine transformations are OK but perspective transformations are not.
    glm::quat bulletToGlm(const btQuaternion &q);

    btQuaternion glmToBullet(const glm::quat &q);

    btMatrix3x3 glmToBullet(const glm::mat3 &m);

    btTransform glmToBullet(const glm::mat4& m);

    glm::mat4 bulletToGlm(const btTransform& t);

    DimensionData GenDimensions(std::vector<glm::vec3> vertices);

    uint32_t SwapEndian(uint32_t x);

    glm::vec3 FixedToFloat(glm::vec3 fixedPoint);

    // TODO: Move to resource handling class
    std::vector<CarModel> LoadOBJ(std::string obj_path);

    bool ExtractVIV(const std::string &viv_path, const std::string &output_dir);

    bool DecompressCRP(const std::string &compressedCrpPath, const std::string &decompressedCrpPath);

    glm::vec3 HSLToRGB(glm::vec4 hsl);

    glm::vec3 ParseRGBString(const std::string &rgb_string);

    glm::vec3 CalculateQuadNormal(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p4);

    glm::vec3 CalculateNormal(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3);

    btTransform MakeTransform(glm::vec3 position, glm::quat orientation);

    // Easily convert propietary and platform specific Vertices to glm::vec3. NFS2_DATA::PC::GEO::BLOCK_3D, NFS2_DATA::PS1::GEO::BLOCK_3D, NFS3_4_DATA::FLOATPT etc.
    template <class NFSVertexStruct>
    glm::vec3 PointToVec(NFSVertexStruct block_3d){
        return glm::vec3(block_3d.x, block_3d.y, block_3d.z);
    }
}
