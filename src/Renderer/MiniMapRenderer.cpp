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

    MiniMapRenderer::~MiniMapRenderer() {
        m_miniMapShader.cleanup();
    }

    void MiniMapRenderer::Render(std::shared_ptr<Track> const &track, std::vector<std::shared_ptr<CarAgent>> const &racers) {
        this->_Rescale();

        m_miniMapShader.use();
        m_miniMapShader.loadProjectionMatrix(projectionMatrix);
        m_miniMapShader.loadColour(glm::vec3(1.f, 1.f, 1.f));

        glm::mat4 const rotationMat{glm::rotate(glm::mat4(1.f), glm::radians(270.f), glm::vec3(-1.0, 0.0, 0.0))};

        for (auto const &trackBlockEntities : track->perTrackblockEntities) {
            for (auto const &entity : trackBlockEntities) {
                if (entity->Type() != LibOpenNFS::EntityType::ROAD) {
                    continue;
                }
                m_miniMapShader.loadTransformationMatrix(rotationMat * entity->ModelMatrix);
                m_miniMapShader.loadLayer(1);
                entity->Render();
            }
        }

        for (auto &racer : racers) {
            m_miniMapShader.loadColour(racer->vehicle->vehicleState.colour);
            auto racerMatrix{
                glm::scale(rotationMat * racer->vehicle->carBodyModel.ModelMatrix, glm::vec3(kVehicleScale, kVehicleScale, kVehicleScale))};
            m_miniMapShader.loadTransformationMatrix(racerMatrix);
            m_miniMapShader.loadLayer(0);
            racer->vehicle->carBodyModel.Render();
        }

        m_miniMapShader.unbind();
        m_miniMapShader.HotReload();
    }
} // namespace OpenNFS
