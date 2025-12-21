#include "CarCamera.h"

namespace OpenNFS {
    CarCamera::CarCamera(InputManager const &inputManager) : BaseCamera(CameraMode::FOLLOW_CAR, inputManager) {
    }

    void CarCamera::FollowCar(std::shared_ptr<Car> const &targetCar) {
        // Blessed be ThinMatrix
        this->_CalculateZoom();
        this->_CalculatePitch();
        this->_CalculateAngleAroundCar();
        this->_CalculateCameraPosition(targetCar, this->_CalculateHorizontalDistance(), this->_CalculateVerticalDistance());
        m_yaw = 180 - ((targetCar->GetCarBodyOrientation() + m_angleAroundCar));

        viewMatrix = glm::mat4(1.0f);
        viewMatrix = glm::rotate(viewMatrix, glm::radians(m_pitch), glm::vec3(1, 0, 0));
        viewMatrix = glm::rotate(viewMatrix, glm::radians(m_yaw), glm::vec3(0, 1, 0));
        glm::vec3 const negativeCameraPos(-position);
        viewMatrix = glm::translate(viewMatrix, negativeCameraPos);
    }

    void CarCamera::_CalculateCameraPosition(std::shared_ptr<Car> const &target_car, float const horizDistance, float const vertDistance) {
        float const theta{(target_car->GetCarBodyOrientation() + m_angleAroundCar)};
        float const offsetX{horizDistance * sin(glm::radians(theta))};
        float const offsetZ{horizDistance * cos(glm::radians(theta))};
        position.x = target_car->carBodyModel.position.x - offsetX;
        position.z = target_car->carBodyModel.position.z - offsetZ;
        position.y = target_car->carBodyModel.position.y + vertDistance;
    }

    void CarCamera::_CalculateZoom() {
        float const zoomLevel{static_cast<float>(m_inputManager.scrollY) * .1f};
        m_distanceFromCar -= zoomLevel;
    }

    void CarCamera::_CalculatePitch() {
        if (m_inputManager.mouseRight) {
            float const pitchChange{static_cast<float>(m_inputManager.mouseDeltaY) * 0.1f};
            m_pitch -= pitchChange;
        }
    }

    void CarCamera::_CalculateAngleAroundCar() {
        if (m_inputManager.mouseLeft) {
            float const angleChange{static_cast<float>(m_inputManager.mouseDeltaX) * 0.3f};
            m_angleAroundCar -= angleChange;
        }
    }

    float CarCamera::_CalculateVerticalDistance() const {
        return m_distanceFromCar * sin(glm::radians(m_pitch));
    }

    float CarCamera::_CalculateHorizontalDistance() const {
        return m_distanceFromCar * cos(glm::radians(m_pitch));
    }
} // namespace OpenNFS
