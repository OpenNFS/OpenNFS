#include "VehicleSelection.h"

namespace OpenNFS {
    VehicleSelection::VehicleSelection(std::shared_ptr<GLFWwindow> const &window, std::vector<NfsAssetList> const &installedNFS,
                                       std::shared_ptr<Car> const &currentCar)
        : m_carCamera(m_inputManager), m_currentCar(currentCar), m_showroomLight({}, {}), m_inputManager(window) {

        // Enable carousel mode with initial angular velocity of 20 degrees/second
        m_carCamera.EnableCarouselMode(20.f);
        m_showroomLight.colour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
        m_showroomLight.ChangeTarget(m_currentCar->carBodyModel.position);
        m_currentCar->lights.push_back(&m_showroomLight);
        m_carouselTransform.setOrigin(btVector3(0, 0, 0));
        m_carouselTransform.setRotation(btQuaternion(0, 0, 0, 1));
    }

    void VehicleSelection::_UpdateCameras(float const deltaTime) {
        m_carCamera.FollowCar(m_currentCar, deltaTime);
    }

    BaseCamera &VehicleSelection::_GetActiveCamera() {
        return m_carCamera;
    }

    void VehicleSelection::Update(float const deltaTime) {
        // Clear the screen for next input and grab focus
        m_inputManager.Scan();

        // Update Cameras
        this->_UpdateCameras(deltaTime);

        // Set the active camera dependent upon user input and update Frustum
        auto &activeCamera{this->_GetActiveCamera()};

        // Draw the Game UI
        m_uiManager.Update(m_inputManager);

        m_currentCar->UpdateMeshesToTransform(m_carouselTransform, true);

        m_carRenderer.Render(m_currentCar, activeCamera, &m_showroomLight, 0);
    }

    bool VehicleSelection::IsFinished() const {
        return false;
    }
} // namespace OpenNFS
