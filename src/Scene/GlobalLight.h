#pragma once

#include <array>

#include "../Config.h"
#include "Entities/BaseLight.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

using namespace LibOpenNFS;

namespace OpenNFS {
    class BaseCamera;

    constexpr float RADIANS_PER_TICK = 0.001f;

    struct CascadeData {
        std::array<float, CSM_NUM_CASCADES> splitDistances{};
        std::array<glm::mat4, CSM_NUM_CASCADES> lightSpaceMatrices{};
        std::array<float, CSM_NUM_CASCADES> cascadePlaneDistances{};
    };

    class GlobalLight final : public BaseLight {
      public:
        GlobalLight();
        explicit GlobalLight(glm::vec3 lookAt, glm::vec3 position);
        void Update(float timeScaleFactor);
        void UpdateCascades(BaseCamera const &camera);
        void ChangeTarget(glm::vec3 lookAt);

        glm::vec3 initialPosition;
        glm::mat4 lightSpaceMatrix;
        glm::mat4 viewMatrix;
        CascadeData cascadeData;

      private:
        void CalculateSplitDistances(float nearPlane, float farPlane, float lambda);
        glm::mat4 CalculateLightSpaceMatrix(glm::mat4 const &cameraView, glm::mat4 const &cameraProj, float nearSplit, float farSplit);
        std::array<glm::vec4, 8> GetFrustumCornersWorldSpace(glm::mat4 const &viewProj);

        glm::mat4 m_lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, 160.f, 300.f);
        glm::vec3 m_lookAt;
    };
} // namespace OpenNFS