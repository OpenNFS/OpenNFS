#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../Camera/CarCamera.h"
#include "../Camera/FreeCamera.h"
#include "../Config.h"
#include "../Input/InputManager.h"
#include "../Physics/PhysicsManager.h"
#include "../Renderer/Renderer.h"

namespace OpenNFS {
    class VehicleSelection {
      public:
        VehicleSelection(std::shared_ptr<GLFWwindow> const &window, std::vector<NfsAssetList> const &installedNFS,
                         std::shared_ptr<Car> const &currentCar);

        void Update(float deltaTime);
        [[nodiscard]] bool IsFinished() const;

      private:
        BaseCamera &_GetActiveCamera();
        void _UpdateCameras(float deltaTime);

        CarCamera m_carCamera;
        std::shared_ptr<Car> m_currentCar;
        btTransform m_carouselTransform{};
        GlobalLight m_showroomLight;
        std::vector<BaseLight const *> m_showroomLights;

        CarRenderer m_carRenderer;
        InputManager m_inputManager;
        UIManager m_uiManager;
    };
} // namespace OpenNFS
