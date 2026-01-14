#include "DebugRenderer.h"

#include "../Race/Agents/RacerAgent.h"
#include "Entities/TrackVRoad.h"
#include "NFS3/NFS3Loader.h"

namespace OpenNFS {
    DebugRenderer::DebugRenderer(std::shared_ptr<BulletDebugDrawer> const &bulletDebugDrawer) : m_bulletDebugDrawer(bulletDebugDrawer) {
    }

    void DebugRenderer::Render(BaseCamera const &camera) const {
        m_bulletDebugDrawer->Render(camera);
    }

    void DebugRenderer::DrawTrackCollision(std::shared_ptr<Track> const &track) const {
        for (auto const &trackBlockEntities : track->perTrackblockEntities) {
            for (auto const &entity : trackBlockEntities) {
                this->DrawAABB(entity->GetAABB(), entity->GetDebugColour());
            }
        }
    }

    void DebugRenderer::DrawAABB(const AABB &aabb, glm::vec3 const &colour) const {
        m_bulletDebugDrawer->drawBox(Utils::glmToBullet(aabb.min), Utils::glmToBullet(aabb.max), Utils::glmToBullet(colour));
    }

    void DebugRenderer::DrawFrustum(BaseCamera const &camera) const {
        std::array<glm::vec3, 8> const frustumDebugVizPoints{camera.viewFrustum.points};

        btVector3 const colour(0, 1, 0);
        // Far Plane
        m_bulletDebugDrawer->drawLine(Utils::glmToBullet(frustumDebugVizPoints[0]), Utils::glmToBullet(frustumDebugVizPoints[1]), colour);
        m_bulletDebugDrawer->drawLine(Utils::glmToBullet(frustumDebugVizPoints[1]), Utils::glmToBullet(frustumDebugVizPoints[2]), colour);
        m_bulletDebugDrawer->drawLine(Utils::glmToBullet(frustumDebugVizPoints[2]), Utils::glmToBullet(frustumDebugVizPoints[3]), colour);
        m_bulletDebugDrawer->drawLine(Utils::glmToBullet(frustumDebugVizPoints[3]), Utils::glmToBullet(frustumDebugVizPoints[0]), colour);

        // Near Plane
        m_bulletDebugDrawer->drawLine(Utils::glmToBullet(frustumDebugVizPoints[4]), Utils::glmToBullet(frustumDebugVizPoints[5]), colour);
        m_bulletDebugDrawer->drawLine(Utils::glmToBullet(frustumDebugVizPoints[5]), Utils::glmToBullet(frustumDebugVizPoints[6]), colour);
        m_bulletDebugDrawer->drawLine(Utils::glmToBullet(frustumDebugVizPoints[6]), Utils::glmToBullet(frustumDebugVizPoints[7]), colour);
        m_bulletDebugDrawer->drawLine(Utils::glmToBullet(frustumDebugVizPoints[7]), Utils::glmToBullet(frustumDebugVizPoints[4]), colour);

        // Near to Far edges
        m_bulletDebugDrawer->drawLine(Utils::glmToBullet(frustumDebugVizPoints[4]), Utils::glmToBullet(frustumDebugVizPoints[0]), colour);
        m_bulletDebugDrawer->drawLine(Utils::glmToBullet(frustumDebugVizPoints[5]), Utils::glmToBullet(frustumDebugVizPoints[1]), colour);
        m_bulletDebugDrawer->drawLine(Utils::glmToBullet(frustumDebugVizPoints[6]), Utils::glmToBullet(frustumDebugVizPoints[2]), colour);
        m_bulletDebugDrawer->drawLine(Utils::glmToBullet(frustumDebugVizPoints[7]), Utils::glmToBullet(frustumDebugVizPoints[3]), colour);
    }

    void DebugRenderer::DrawVehicleRaycasts(std::shared_ptr<Car> const &car) const {
        glm::vec3 const carBodyPosition{car->carBodyModel.position};
        for (uint8_t rangeIdx = 0; rangeIdx < kNumRangefinders; ++rangeIdx) {
            m_bulletDebugDrawer->drawLine(Utils::glmToBullet(carBodyPosition),
                                          Utils::glmToBullet(car->rangefinderInfo.castPositions[rangeIdx]),
                                          btVector3(2.0f * (kFarDistance - car->rangefinderInfo.rangefinders[rangeIdx]),
                                                    2.0f * (car->rangefinderInfo.rangefinders[rangeIdx]), 0));
        }

        // Draw up and down casts
        m_bulletDebugDrawer->drawLine(
            Utils::glmToBullet(carBodyPosition), Utils::glmToBullet(car->rangefinderInfo.upCastPosition),
            btVector3(2.0f * (kFarDistance - car->rangefinderInfo.upDistance), 2.0f * (car->rangefinderInfo.upDistance), 0));
        m_bulletDebugDrawer->drawLine(
            Utils::glmToBullet(carBodyPosition), Utils::glmToBullet(car->rangefinderInfo.downCastPosition),
            btVector3(2.0f * (kFarDistance - car->rangefinderInfo.downDistance), 2.0f * (car->rangefinderInfo.downDistance), 0));

        for (auto &light : car->lights) {
            auto const spotlight{((Spotlight *&)light)};
            DrawDummy(spotlight->position, spotlight->direction);
        }
    }

    void DebugRenderer::DrawVehicleAI(std::shared_ptr<RacerAgent> const &racer, BaseCamera const &camera) const {
        // Line from vehicle body to target vroad
        glm::vec3 const carBodyPosition{racer->vehicle->carBodyModel.position};
        m_bulletDebugDrawer->drawLine(Utils::glmToBullet(carBodyPosition), Utils::glmToBullet(racer->targetVroadPosition),
                                      btVector3(0, 0, 1));

        glm::vec3 namePos{carBodyPosition};
        namePos.y += 0.1f; // Height offset above car

        // Calculate distance-based scale
        float const distance = glm::length(camera.position - namePos);
        float const scale = 0.00015f * distance;                     // Scale with distance
        float const clampedScale = glm::clamp(scale, 0.001f, 0.15f); // Clamp for readability

        // Render the racer name and state
        std::string const racerStatusText{racer->name + ": " + std::string(magic_enum::enum_name(racer->State()))};
        m_text3DRenderer.RenderText3D(racerStatusText, namePos, camera, clampedScale, glm::vec4(1, 1, 1, 1));
    }

    void DebugRenderer::DrawDummy(glm::vec3 const position, glm::vec3 const direction) const {
        constexpr float kDummySize{0.1f};
        auto const btPosition{Utils::glmToBullet(position)};
        auto const btDirection{Utils::glmToBullet(direction)};
        m_bulletDebugDrawer->drawLine(btPosition, btPosition + (btDirection * kDummySize),
                                      Utils::glmToBullet(glm::vec3(0, 62, 80) / 255.f));
    }

    void DebugRenderer::DrawVroad(std::shared_ptr<Track> const &track) const {
        float const vRoadDisplayHeight = 0.2f;
        uint32_t const nVroad = track->virtualRoad.size();

        for (uint32_t vroadIdx = 0; vroadIdx < nVroad; ++vroadIdx) {
            // Render COL Vroad? Should I use TRK VROAD to work across HS too?
            if (vroadIdx < nVroad - 1) {
                LibOpenNFS::TrackVRoad const &curVroad = track->virtualRoad[vroadIdx];
                LibOpenNFS::TrackVRoad const &nextVroad = track->virtualRoad[vroadIdx + 1];

                glm::vec3 vroadPoint = curVroad.position;
                glm::vec3 vroadPointNext = nextVroad.position;

                // Add a little vertical offset so it's not clipping through track geometry
                vroadPoint.y += vRoadDisplayHeight;
                vroadPointNext.y += vRoadDisplayHeight;
                m_bulletDebugDrawer->drawLine(Utils::glmToBullet(vroadPoint), Utils::glmToBullet(vroadPointNext), btVector3(1, 0, 1));
                m_bulletDebugDrawer->drawLine(Utils::glmToBullet(vroadPoint), Utils::glmToBullet(vroadPoint + curVroad.forward),
                                              btVector3(0, 1, 0));
                m_bulletDebugDrawer->drawLine(Utils::glmToBullet(vroadPoint), Utils::glmToBullet(vroadPoint + curVroad.normal),
                                              btVector3(0, 0, 1));
                m_bulletDebugDrawer->drawLine(Utils::glmToBullet(vroadPoint), Utils::glmToBullet(vroadPoint - curVroad.leftWall),
                                              btVector3(1, 0, 0.5f));
                m_bulletDebugDrawer->drawLine(Utils::glmToBullet(vroadPoint), Utils::glmToBullet(vroadPoint + curVroad.rightWall),
                                              btVector3(1, 0, 0.5f));
            }
        }
    }

    void DebugRenderer::DrawCameraAnimation(std::shared_ptr<Track> const &track) const {
        using namespace LibOpenNFS;

        for (size_t canIdx = 0; canIdx < track->cameraAnimation.size() - 1; ++canIdx) {
            // Draw CAN positions
            Shared::CameraAnimPoint const refPt = track->cameraAnimation[canIdx];
            Shared::CameraAnimPoint const refPtNext = track->cameraAnimation[canIdx + 1];
            glm::vec3 vroadPoint{LibOpenNFS::Utils::FixedToFloat(refPt.pt) * NFS3::NFS3_SCALE_FACTOR};
            glm::vec3 vroadPointNext{LibOpenNFS::Utils::FixedToFloat(refPtNext.pt) * NFS3::NFS3_SCALE_FACTOR};
            vroadPoint.y += 0.2f;
            vroadPointNext.y += 0.2f;
            m_bulletDebugDrawer->drawLine(Utils::glmToBullet(vroadPoint + track->trackBlocks[0].position),
                                          Utils::glmToBullet(vroadPointNext + track->trackBlocks[0].position), btVector3(0, 1, 1));

            // Draw Rotations (probably interpreted incorrectly)
            glm::quat RotationMatrix{glm::normalize(glm::quat((refPt.od1), refPt.od2, refPt.od3, refPt.od4))};
            glm::vec3 direction = glm::normalize(vroadPoint * RotationMatrix);
            m_bulletDebugDrawer->drawLine(Utils::glmToBullet(vroadPoint + track->trackBlocks[0].position),
                                          Utils::glmToBullet(vroadPoint + track->trackBlocks[0].position + direction),
                                          btVector3(0, 0.5, 0.5));
        }
    }
} // namespace OpenNFS
