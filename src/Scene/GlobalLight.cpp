#include "GlobalLight.h"
#include "../Camera/BaseCamera.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/quaternion.hpp"

#include <cmath>
#include <limits>

namespace OpenNFS {
    GlobalLight::GlobalLight() : BaseLight(0, 0, GLOBAL_LIGHT, glm::vec3(), glm::vec4()) {
    }

    GlobalLight::GlobalLight(glm::vec3 const lookAt, glm::vec3 const position)
        : BaseLight(0, 0, GLOBAL_LIGHT, position, glm::vec4(0, 0, 0, 0)) {
        this->initialPosition = position;
        this->ChangeTarget(lookAt);
        this->Update(0.f);
    }
    void GlobalLight::ChangeTarget(glm::vec3 const lookAt) {
        m_lookAt = lookAt;
    }

    void GlobalLight::Update(float const timeScaleFactor) {
        position = position * glm::normalize(glm::quat(glm::vec3(timeScaleFactor * RADIANS_PER_TICK, 0, 0)));
        viewMatrix = glm::lookAt(position, m_lookAt, glm::vec3(0, 1.0, 0));
        lightSpaceMatrix = m_lightProjection * viewMatrix;
    }

    void GlobalLight::CalculateSplitDistances(float const nearPlane, float const farPlane, float const lambda) {
        float const ratio = farPlane / nearPlane;

        for (uint32_t i = 0; i < CSM_NUM_CASCADES; ++i) {
            float const p = static_cast<float>(i + 1) / static_cast<float>(CSM_NUM_CASCADES);

            // Logarithmic split (better distribution for perspective)
            float const logSplit = nearPlane * std::pow(ratio, p);

            // Uniform split (linear distribution)
            float const uniformSplit = nearPlane + (farPlane - nearPlane) * p;

            // Practical split: blend between log and uniform using lambda
            cascadeData.splitDistances[i] = lambda * logSplit + (1.0f - lambda) * uniformSplit;
            cascadeData.cascadePlaneDistances[i] = cascadeData.splitDistances[i];
        }
    }

    std::array<glm::vec4, 8> GlobalLight::GetFrustumCornersWorldSpace(glm::mat4 const &viewProj) {
        glm::mat4 const inverse = glm::inverse(viewProj);

        std::array<glm::vec4, 8> frustumCorners;
        int index = 0;

        for (int x = 0; x < 2; ++x) {
            for (int y = 0; y < 2; ++y) {
                for (int z = 0; z < 2; ++z) {
                    glm::vec4 const pt = inverse * glm::vec4(2.0f * x - 1.0f, 2.0f * y - 1.0f, 2.0f * z - 1.0f, 1.0f);
                    frustumCorners[index++] = pt / pt.w;
                }
            }
        }

        return frustumCorners;
    }

    glm::mat4 GlobalLight::CalculateLightSpaceMatrix(glm::mat4 const &cameraView, glm::mat4 const &cameraProj, float const nearSplit,
                                                     float const farSplit) {
        // Extract FOV and aspect from camera projection matrix
        float const tanHalfFovY = 1.0f / cameraProj[1][1];
        float const aspect = cameraProj[1][1] / cameraProj[0][0];

        // Create a projection matrix for just this cascade's range
        glm::mat4 const cascadeProj = glm::perspective(2.0f * std::atan(tanHalfFovY), aspect, nearSplit, farSplit);

        glm::mat4 const cascadeViewProj = cascadeProj * cameraView;
        auto const corners = GetFrustumCornersWorldSpace(cascadeViewProj);

        // Calculate frustum center
        glm::vec3 center = glm::vec3(0.0f);
        for (auto const &corner : corners) {
            center += glm::vec3(corner);
        }
        center /= 8.0f;

        // Light view matrix looking at frustum center
        glm::vec3 const lightDir = glm::normalize(position);
        glm::mat4 const lightView = glm::lookAt(center + lightDir * 100.0f, center, glm::vec3(0.0f, 1.0f, 0.0f));

        // Transform frustum corners to light space and find bounding box
        float minX = std::numeric_limits<float>::max();
        float maxX = std::numeric_limits<float>::lowest();
        float minY = std::numeric_limits<float>::max();
        float maxY = std::numeric_limits<float>::lowest();
        float minZ = std::numeric_limits<float>::max();
        float maxZ = std::numeric_limits<float>::lowest();

        for (auto const &corner : corners) {
            glm::vec4 const lightSpaceCorner = lightView * corner;
            minX = std::min(minX, lightSpaceCorner.x);
            maxX = std::max(maxX, lightSpaceCorner.x);
            minY = std::min(minY, lightSpaceCorner.y);
            maxY = std::max(maxY, lightSpaceCorner.y);
            minZ = std::min(minZ, lightSpaceCorner.z);
            maxZ = std::max(maxZ, lightSpaceCorner.z);
        }

        // Add margin for shadow casters outside visible frustum
        constexpr float zMargin = 150.0f;
        minZ -= zMargin;

        // Create tight orthographic projection
        glm::mat4 const lightProj = glm::ortho(minX, maxX, minY, maxY, -maxZ, -minZ);

        return lightProj * lightView;
    }

    void GlobalLight::UpdateCascades(BaseCamera const &camera) {
        CalculateSplitDistances(CSM_NEAR_PLANE, CSM_FAR_PLANE, CSM_LAMBDA);

        float nearSplit = CSM_NEAR_PLANE;

        for (uint32_t i = 0; i < CSM_NUM_CASCADES; ++i) {
            float const farSplit = cascadeData.splitDistances[i];

            cascadeData.lightSpaceMatrices[i] = CalculateLightSpaceMatrix(camera.viewMatrix, camera.projectionMatrix, nearSplit, farSplit);

            nearSplit = farSplit;
        }

        // Keep backward compatibility - use first cascade for legacy code
        lightSpaceMatrix = cascadeData.lightSpaceMatrices[0];
    }

} // namespace OpenNFS
