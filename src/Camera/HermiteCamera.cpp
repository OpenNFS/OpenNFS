#include "HermiteCamera.h"

HermiteCamera::HermiteCamera(const HermiteCurve &trackCenterSpline, glm::vec3 initialPosition, GLFWwindow *window) :
Camera(CameraMode::HERMITE_FLYTHROUGH, initialPosition, window),
m_trackCameraRail(trackCenterSpline)
{
    m_loopTime = static_cast<int>(m_trackCameraRail.GetLength()) * 100;
}

void HermiteCamera::UseSpline(float elapsedTime) {
    // Ensure we're never sampling the hermite curve outside of points arr size.
    float tmod = fmod(elapsedTime, (m_loopTime / 202.5f)) / (m_loopTime / 200.f);
    glm::vec3 hermitePosition = m_trackCameraRail.GetPointAt(tmod);

    // Look towards the position that is a few ms away
    float tmodLookAt = tmod + 0.01f;
    glm::vec3 lookAtPos = m_trackCameraRail.GetPointAt(tmodLookAt);
    glm::vec3 hermiteDirection = glm::normalize(lookAtPos - position);

    // https://github.com/phoboslab/wipeout/blob/master/wipeout.js [Wipeout.prototype.updateSplineCamera]
    // Roll into corners - there's probably an easier way to do this. This
    // takes the angle between the current camera position and the current
    // lookAt, applies some damping and rolls the camera along its view vector
    glm::vec3 cn = hermitePosition - lookAtPos;
    glm::vec3 tn = hermitePosition;
    float newRoll = (atan2(cn.z, cn.x) - atan2(tn.z, tn.x));
    newRoll += (newRoll > SIMD_PI) ? -SIMD_PI * 2 : (newRoll < -SIMD_PI) ? SIMD_PI * 2 : 0;
    m_roll = m_roll * 0.95f + (newRoll) * 0.1f;

    // Create a new 'up' vector, based on the roll value
    glm::vec3 up =
            glm::rotate(glm::mat4(1), (m_roll * 0.25f) + 0.75f, hermitePosition) * glm::vec4(glm::vec3(0, 1, 0), 1.0);

    // Camera matrix
    viewMatrix = glm::lookAt(
            hermitePosition,             // Camera is here
            hermitePosition + hermiteDirection, // and looks here : at the same position, plus "direction"
            up
    );

    this->_UpdateFrustum();
}