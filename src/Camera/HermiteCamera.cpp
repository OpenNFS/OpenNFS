#include "HermiteCamera.h"

#include "glm/detail/type_quat.hpp"
#include "glm/ext/quaternion_trigonometric.hpp"

namespace OpenNFS {
    HermiteCamera::HermiteCamera(HermiteCurve const &trackCenterSpline, InputManager const &inputManager)
        : BaseCamera(CameraMode::HERMITE_FLYTHROUGH, inputManager), m_trackCameraRail(trackCenterSpline) {
        m_loopTime = static_cast<int>(m_trackCameraRail.GetLength()) * 100;
    }

    void HermiteCamera::UseSpline(float const elapsedTime) {
        // Ensure we're never sampling the hermite curve outside of points arr size.
        float const tmod{fmodf(elapsedTime, (m_loopTime / 202.5f)) / (m_loopTime / 200.f)};
        position = m_trackCameraRail.GetPointAt(tmod);

        // Look towards the position that is a few ms ahead
        float const tmodLookAt{tmod + 0.01f};
        glm::vec3 const lookAtPos{m_trackCameraRail.GetPointAt(tmodLookAt)};
        m_direction = glm::normalize(lookAtPos - position);

        // Calculate roll based on lateral change in direction (banking into turns)
        // Get a point slightly further ahead to calculate the turn direction
        float const tmodFuture{tmod + 0.02f};
        glm::vec3 const futurePos{m_trackCameraRail.GetPointAt(tmodFuture)};
        glm::vec3 const futureDir{glm::normalize(futurePos - lookAtPos)};

        // Calculate the lateral (right) vector
        glm::vec3 const worldUp{0.0f, 1.0f, 0.0f};
        glm::vec3 const right{glm::normalize(glm::cross(m_direction, worldUp))};

        // Measure how much the direction is changing laterally (turning)
        glm::vec3 const directionChange{futureDir - m_direction};
        float const lateralChange{glm::dot(directionChange, right)};

        // Calculate new roll with damping, and clamp to prevent flipping
        float const rollAmount{lateralChange * 2.0f};
        float const targetRoll{glm::clamp(rollAmount, -0.5f, 0.5f)}; // Clamp to ±0.5 radians (~±28 degrees)
        m_roll = m_roll * 0.9f + targetRoll * 0.1f; // Smooth interpolation

        // Create the up vector by rotating world up around the forward direction
        glm::quat const rollQuat{glm::angleAxis(m_roll, m_direction)};
        glm::vec3 const up{rollQuat * worldUp};

        // Camera matrix
        viewMatrix = glm::lookAt(position,               // Camera is here
                                 position + m_direction, // and looks here
                                 up);
    }
} // namespace OpenNFS
