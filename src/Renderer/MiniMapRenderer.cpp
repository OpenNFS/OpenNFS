#include "MiniMapRenderer.h"

#include "../Physics/Car.h"
#include "../Race/Agents/CarAgent.h"

namespace OpenNFS {
    void MiniMapRenderer::_Rescale() {
        left = -200.f;
        right = 800.f;
        bottom = -200.f;
        top = 800.f;
        projectionMatrix = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);
    }

    void MiniMapRenderer::Render(Track const &track, std::vector<std::shared_ptr<CarAgent>> const &racers) {
        this->_Rescale();

        miniMapShader.use();
        miniMapShader.loadProjectionMatrix(projectionMatrix);
        miniMapShader.loadColor(glm::vec3(1.f, 1.f, 1.f));

        glm::mat4 const rotationMat {glm::rotate(glm::mat4(1.f), glm::radians(270.f), glm::vec3(1.0, 0.0, 0.0))};

        for (auto &trackEntity : track.entities) {
            if (trackEntity->type != LibOpenNFS::EntityType::ROAD) {
                continue;
            }
            miniMapShader.loadTransformationMatrix(rotationMat * trackEntity->ModelMatrix);
            miniMapShader.loadLayer(1);
            trackEntity->Render();
        }

        for (auto &racer : racers) {
            miniMapShader.loadColor(racer->vehicle->vehicleState.colour);
            auto racerMatrix {glm::scale(rotationMat * racer->vehicle->carBodyModel.ModelMatrix, glm::vec3(kVehicleScale, kVehicleScale, kVehicleScale))};
            miniMapShader.loadTransformationMatrix(racerMatrix);
            miniMapShader.loadLayer(0);
            racer->vehicle->carBodyModel.Render();
        }

        miniMapShader.unbind();
        miniMapShader.HotReload();
    }
} // namespace OpenNFS
