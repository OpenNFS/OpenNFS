#include "Camera.h"
#include <glm/gtx/quaternion.hpp>

Camera::Camera(glm::vec3 initialPosition, HermiteCurve trackCenterSpline, GLFWwindow *window) {
    m_window = window;
    m_fov = Config::get().fov;
    // Initial position : on +Z
    position = initialPosition;
    m_initialPosition = initialPosition;
    // Projection matrix : 45deg Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    projectionMatrix = glm::perspective(glm::radians(m_fov), 4.0f / 3.0f, 0.1f, 1000.0f);
    // Pull in track spline for easy access
    m_trackCameraRail = trackCenterSpline;
    m_loopTime = static_cast<int>(m_trackCameraRail.GetLength()) * 100;
}

void Camera::ResetView() {
    position = m_initialPosition;
    m_horizontalAngle = 3.14f;
    m_verticalAngle = 0.0f;
    m_direction = glm::vec3 (
            cos(m_verticalAngle) * sin(m_horizontalAngle),
            sin(m_verticalAngle),
            cos(m_verticalAngle) * cos(m_horizontalAngle)
    );
    glm::vec3 right = glm::vec3(
            sin(m_horizontalAngle - 3.14f / 2.0f),
            0,
            cos(m_horizontalAngle - 3.14f / 2.0f)
    );
    // Camera matrix
    viewMatrix = glm::lookAt(
            position,
            position + m_direction,
            glm::cross(right, m_direction)
    );
}

void Camera::UseSpline(float elapsedTime) {
    // Ensure we're never sampling the hermite curve outside of points arr size.
    float tmod = fmod(elapsedTime, (m_loopTime / 202.5f)) / (m_loopTime / 200.f);
    position = m_trackCameraRail.GetPointAt(tmod);

    // Look towards the position that is a few ms away
    float tmodLookAt = tmod + 0.01f;
    glm::vec3 lookAtPos  = m_trackCameraRail.GetPointAt(tmodLookAt);
    m_direction = glm::normalize(lookAtPos - position);

    // https://github.com/phoboslab/wipeout/blob/master/wipeout.js [Wipeout.prototype.updateSplineCamera]
    // Roll into corners - there's probably an easier way to do this. This
    // takes the angle between the current camera position and the current
    // lookAt, applies some damping and rolls the camera along its view vector
    glm::vec3 cn = position - lookAtPos;
    glm::vec3 tn = position;
    float newRoll = (atan2(cn.z, cn.x) - atan2(tn.z, tn.x));
    newRoll += (newRoll > SIMD_PI) ? -SIMD_PI *2 : (newRoll < -SIMD_PI) ? SIMD_PI * 2 : 0;
    m_roll = m_roll * 0.95f + (newRoll) * 0.1f;

    // Create a new 'up' vector, based on the roll value
    glm::vec3 up = glm::rotate(glm::mat4(1), (m_roll * 0.25f) + 0.75f, m_direction) * glm::vec4(glm::vec3(0, 1, 0), 1.0);

    // Camera matrix
    viewMatrix = glm::lookAt(
            position,             // Camera is here
            position + m_direction, // and looks here : at the same position, plus "direction"
            up
    );
}

void Camera::CalculateCameraPosition(const std::shared_ptr<Car> &target_car, float horizDistance, float vertDistance) {
    float theta =  (target_car->getRotY() + m_angleAroundCar);
    float offsetX = horizDistance * sin(glm::radians(theta));
    float offsetZ = horizDistance * cos(glm::radians(theta));
    position.x = target_car->carBodyModel.position.x - offsetX;
    position.z = target_car->carBodyModel.position.z - offsetZ;
    position.y = target_car->carBodyModel.position.y + vertDistance;
}

void Camera::CalculateZoom() {
    float zoomLevel = ImGui::GetIO().MouseWheel * 0.1f;
    m_distanceFromCar -= zoomLevel;
}

void Camera::CalculatePitch() {
    if (ImGui::GetIO().MouseDown[1]) {
        float pitchChange = ImGui::GetIO().MouseDelta.y * 0.1f;
        m_pitch -= pitchChange;
    }
}

void Camera::CalculateAngleAroundCar() {
    if (ImGui::GetIO().MouseDown[0]) {
        float angleChange = ImGui::GetIO().MouseDelta.x * 0.3f;
        m_angleAroundCar -= angleChange;
    }
}

float Camera::CalculateVerticalDistance() {
    return m_distanceFromCar * sin(m_pitch * (SIMD_PI / 180.0f));
}

float Camera::CalculateHorizontalDistance() {
    return m_distanceFromCar * cos(m_pitch * (SIMD_PI / 180));
}

void Camera::FollowCar(const std::shared_ptr<Car> &targetCar, bool &windowActive){
    if (!windowActive)
        return;
    // Bail on the window active status if we hit the escape key
    windowActive = (glfwGetKey(m_window, GLFW_KEY_ESCAPE) != GLFW_PRESS);
    ImGui::GetIO().MouseDrawCursor = true;

    // Blessed be ThinMatrix
    CalculateZoom();
    CalculatePitch();
    CalculateAngleAroundCar();
    float horizontalDistance = CalculateHorizontalDistance();
    float verticalDistance = CalculateVerticalDistance();
    CalculateCameraPosition(targetCar, horizontalDistance, verticalDistance);
    m_yaw = 180 - ((targetCar->getRotY() + m_angleAroundCar));

    viewMatrix = glm::mat4(1.0f);
    viewMatrix = glm::rotate(viewMatrix, m_pitch * SIMD_PI / 180, glm::vec3(1, 0, 0));
    viewMatrix = glm::rotate(viewMatrix, m_yaw * SIMD_PI / 180, glm::vec3(0, 1, 0));
    glm::vec3 negativeCameraPos(-position);
    viewMatrix = glm::translate(viewMatrix, negativeCameraPos);
}

void Camera::ComputeMatricesFromInputs(bool &windowActive, float deltaTime) {
    if (!windowActive)
        return;
    // Bail on the window active status if we hit the escape key
    windowActive = (glfwGetKey(m_window, GLFW_KEY_ESCAPE) != GLFW_PRESS);
    ImGui::GetIO().MouseDrawCursor = true;

    // Get mouse position and compute new orientation with it
    m_horizontalAngle += m_mouseSpeedDamper * (Config::get().resX / 2 - ImGui::GetIO().MousePos.x);
    m_verticalAngle += m_mouseSpeedDamper * (Config::get().resY / 2 - ImGui::GetIO().MousePos.y);

    // Reset mouse position for next frame
    glfwSetCursorPos(m_window, Config::get().resX / 2, Config::get().resY / 2);

    // Direction : Spherical coordinates to Cartesian coordinates conversion
     m_direction = glm::vec3(
             cos(m_verticalAngle) * sin(m_horizontalAngle),
            sin(m_verticalAngle),
             cos(m_verticalAngle) * cos(m_horizontalAngle)
    );

    // Right vector
    glm::vec3 right = glm::vec3(
            sin(m_horizontalAngle - 3.14f / 2.0f),
            0,
            cos(m_horizontalAngle - 3.14f / 2.0f)
    );

    // Up vector
    glm::vec3 up = glm::cross(right, m_direction);

    // Speed boost
    if (ImGui::GetIO().KeyShift) {
        m_speed = 100.0f;
    } else {
        m_speed = 3.0f;
    }

    if (ImGui::GetIO().MouseDown[1]) {
        // Move forward
        if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS) {
            position += m_direction * deltaTime * m_speed;
        }
        // Move backward
        if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS) {
            position -= m_direction * deltaTime * m_speed;
        }
        // Strafe right
        if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS) {
            position += right * deltaTime * m_speed;
        }
        // Strafe left
        if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS) {
            position -= right * deltaTime * m_speed;
        }
    }

    // Camera matrix
    viewMatrix = glm::lookAt(
            position,           // Camera is here
            position + m_direction, // and looks here : at the same position, plus "direction"
            up                  // Head is up (set to 0,-1,0 to look upside-down)
    );
}

Camera::Camera() = default;

