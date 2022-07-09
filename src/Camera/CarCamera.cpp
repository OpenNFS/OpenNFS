#include "CarCamera.h"

CarCamera::CarCamera(const std::shared_ptr<GLFWwindow> &window) : BaseCamera(CameraMode::FOLLOW_CAR, window) {
}

void CarCamera::FollowCar(const std::shared_ptr<Car> &targetCar) {
    // Blessed be ThinMatrix
    this->_CalculateZoom();
    this->_CalculatePitch();
    this->_CalculateAngleAroundCar();
    this->_CalculateCameraPosition(targetCar, this->_CalculateHorizontalDistance(), this->_CalculateVerticalDistance());
    m_yaw = 180 - ((targetCar->GetCarBodyOrientation() + m_angleAroundCar));

    viewMatrix = glm::mat4(1.0f);
    viewMatrix = glm::rotate(viewMatrix, m_pitch * SIMD_PI / 180, glm::vec3(1, 0, 0));
    viewMatrix = glm::rotate(viewMatrix, m_yaw * SIMD_PI / 180, glm::vec3(0, 1, 0));
    glm::vec3 negativeCameraPos(-position);
    viewMatrix = glm::translate(viewMatrix, negativeCameraPos);
}

void CarCamera::_CalculateCameraPosition(const std::shared_ptr<Car> &target_car, float horizDistance, float vertDistance) {
    float theta   = (target_car->GetCarBodyOrientation() + m_angleAroundCar);
    float offsetX = horizDistance * sin(glm::radians(theta));
    float offsetZ = horizDistance * cos(glm::radians(theta));
    position.x    = target_car->carBodyModel.position.x - offsetX;
    position.z    = target_car->carBodyModel.position.z - offsetZ;
    position.y    = target_car->carBodyModel.position.y + vertDistance;
}

void CarCamera::_CalculateZoom() {
    float zoomLevel = ImGui::GetIO().MouseWheel * 0.1f;
    m_distanceFromCar -= zoomLevel;
}

void CarCamera::_CalculatePitch() {
    if (ImGui::GetIO().MouseDown[1]) {
        float pitchChange = ImGui::GetIO().MouseDelta.y * 0.1f;
        m_pitch -= pitchChange;
    }
}

void CarCamera::_CalculateAngleAroundCar() {
    if (ImGui::GetIO().MouseDown[0]) {
        float angleChange = ImGui::GetIO().MouseDelta.x * 0.3f;
        m_angleAroundCar -= angleChange;
    }
}

float CarCamera::_CalculateVerticalDistance() {
    return m_distanceFromCar * sin(m_pitch * (SIMD_PI / 180.0f));
}

float CarCamera::_CalculateHorizontalDistance() {
    return m_distanceFromCar * cos(m_pitch * (SIMD_PI / 180));
}
