#include "DebugRenderer.h"

#include <NFS3/NFS3Loader.h>

#include "Entities/TrackVRoad.h"

namespace OpenNFS {
    DebugRenderer::DebugRenderer(const std::shared_ptr<BulletDebugDrawer> &bulletDebugDrawer) : m_bulletDebugDrawer(
        bulletDebugDrawer) {
    }

    void DebugRenderer::Render(const BaseCamera &camera) const {
        m_bulletDebugDrawer->Render(camera);
    }

    void DebugRenderer::DrawTrackCollision(const Track &track) const {
        for (const auto &trackEntity: track.entities) {
            this->DrawAABB(trackEntity->GetAABB());
        }
    }

    void DebugRenderer::DrawAABB(const AABB &aabb) const {
        btVector3 const colour{0, 0, 0};
        m_bulletDebugDrawer->drawBox(Utils::glmToBullet(aabb.position + aabb.min),
                                     Utils::glmToBullet(aabb.position + aabb.max), colour);
    }

    void DebugRenderer::DrawFrustum(const BaseCamera &camera) const {
        std::array<glm::vec3, 8> const frustumDebugVizPoints{camera.viewFrustum.points};

        btVector3 const colour(0, 1, 0);
        // Far Plane
        m_bulletDebugDrawer->drawLine(Utils::glmToBullet(frustumDebugVizPoints[0]),
                                      Utils::glmToBullet(frustumDebugVizPoints[1]), colour);
        m_bulletDebugDrawer->drawLine(Utils::glmToBullet(frustumDebugVizPoints[1]),
                                      Utils::glmToBullet(frustumDebugVizPoints[2]), colour);
        m_bulletDebugDrawer->drawLine(Utils::glmToBullet(frustumDebugVizPoints[2]),
                                      Utils::glmToBullet(frustumDebugVizPoints[3]), colour);
        m_bulletDebugDrawer->drawLine(Utils::glmToBullet(frustumDebugVizPoints[3]),
                                      Utils::glmToBullet(frustumDebugVizPoints[0]), colour);

        // Near Plane
        m_bulletDebugDrawer->drawLine(Utils::glmToBullet(frustumDebugVizPoints[4]),
                                      Utils::glmToBullet(frustumDebugVizPoints[5]), colour);
        m_bulletDebugDrawer->drawLine(Utils::glmToBullet(frustumDebugVizPoints[5]),
                                      Utils::glmToBullet(frustumDebugVizPoints[6]), colour);
        m_bulletDebugDrawer->drawLine(Utils::glmToBullet(frustumDebugVizPoints[6]),
                                      Utils::glmToBullet(frustumDebugVizPoints[7]), colour);
        m_bulletDebugDrawer->drawLine(Utils::glmToBullet(frustumDebugVizPoints[7]),
                                      Utils::glmToBullet(frustumDebugVizPoints[4]), colour);

        // Near to Far edges
        m_bulletDebugDrawer->drawLine(Utils::glmToBullet(frustumDebugVizPoints[4]),
                                      Utils::glmToBullet(frustumDebugVizPoints[0]), colour);
        m_bulletDebugDrawer->drawLine(Utils::glmToBullet(frustumDebugVizPoints[5]),
                                      Utils::glmToBullet(frustumDebugVizPoints[1]), colour);
        m_bulletDebugDrawer->drawLine(Utils::glmToBullet(frustumDebugVizPoints[6]),
                                      Utils::glmToBullet(frustumDebugVizPoints[2]), colour);
        m_bulletDebugDrawer->drawLine(Utils::glmToBullet(frustumDebugVizPoints[7]),
                                      Utils::glmToBullet(frustumDebugVizPoints[3]), colour);
    }

    void DebugRenderer::DrawCarRaycasts(const std::shared_ptr<Car> &car) const {
        glm::vec3 const carBodyPosition{car->carBodyModel.position};
        for (uint8_t rangeIdx = 0; rangeIdx < kNumRangefinders; ++rangeIdx) {
            m_bulletDebugDrawer->drawLine(Utils::glmToBullet(carBodyPosition),
                                          Utils::glmToBullet(car->rangefinderInfo.castPositions[rangeIdx]),
                                          btVector3(2.0f * (kFarDistance - car->rangefinderInfo.rangefinders[rangeIdx]),
                                                    2.0f * (car->rangefinderInfo.rangefinders[rangeIdx]), 0));
        }

        // Draw up and down casts
        m_bulletDebugDrawer->drawLine(Utils::glmToBullet(carBodyPosition),
                                      Utils::glmToBullet(car->rangefinderInfo.upCastPosition),
                                      btVector3(2.0f * (kFarDistance - car->rangefinderInfo.upDistance),
                                                2.0f * (car->rangefinderInfo.upDistance), 0));
        m_bulletDebugDrawer->drawLine(Utils::glmToBullet(carBodyPosition),
                                      Utils::glmToBullet(car->rangefinderInfo.downCastPosition),
                                      btVector3(2.0f * (kFarDistance - car->rangefinderInfo.downDistance),
                                                2.0f * (car->rangefinderInfo.downDistance), 0));
    }

    void DebugRenderer::DrawVroad(const Track &track) const {
        float vRoadDisplayHeight = 0.2f;
        uint32_t const nVroad = track.virtualRoad.size();

        for (uint32_t vroadIdx = 0; vroadIdx < nVroad; ++vroadIdx) {
            // Render COL Vroad? Should I use TRK VROAD to work across HS too?
            if (vroadIdx < nVroad - 1) {
                LibOpenNFS::TrackVRoad const &curVroad = track.virtualRoad[vroadIdx];
                LibOpenNFS::TrackVRoad const &nextVroad = track.virtualRoad[vroadIdx + 1];

                glm::vec3 vroadPoint = curVroad.position;
                glm::vec3 vroadPointNext = nextVroad.position;

                // Add a little vertical offset so it's not clipping through track geometry
                vroadPoint.y += vRoadDisplayHeight;
                vroadPointNext.y += vRoadDisplayHeight;
                m_bulletDebugDrawer->drawLine(Utils::glmToBullet(vroadPoint), Utils::glmToBullet(vroadPointNext),
                                              btVector3(1, 0, 1));
                m_bulletDebugDrawer->drawLine(Utils::glmToBullet(vroadPoint), Utils::glmToBullet(vroadPointNext),
                                              btVector3(1, 0, 1));

                glm::vec3 curVroadRightVec = curVroad.right;

                if (Config::get().useFullVroad) {
                    m_bulletDebugDrawer->drawLine(Utils::glmToBullet(vroadPoint),
                                                  Utils::glmToBullet(vroadPoint - curVroad.leftWall),
                                                  btVector3(1, 0, 0.5f));
                    m_bulletDebugDrawer->drawLine(Utils::glmToBullet(vroadPoint),
                                                  Utils::glmToBullet(vroadPoint + curVroad.rightWall),
                                                  btVector3(1, 0, 0.5f));
                } else {
                    m_bulletDebugDrawer->drawLine(Utils::glmToBullet(vroadPoint),
                                                  Utils::glmToBullet(vroadPoint + curVroadRightVec),
                                                  btVector3(1, 0, 0.5f));
                    m_bulletDebugDrawer->drawLine(Utils::glmToBullet(vroadPoint),
                                                  Utils::glmToBullet(vroadPoint - curVroadRightVec),
                                                  btVector3(1, 0, 0.5f));
                }
            }
        }
    }

    void DebugRenderer::DrawCameraAnimation(const Track &track) const {
        using namespace LibOpenNFS;

        for (uint8_t canIdx = 0; canIdx < track.cameraAnimation.size() - 1; ++canIdx)
        {
            glm::quat rotationMatrix = glm::normalize(glm::quat(glm::vec3(-glm::pi<float>() / 2, 0, 0)));

            // Draw CAN positions
            Shared::CameraAnimPoint refPt = track.cameraAnimation[canIdx];
            Shared::CameraAnimPoint refPtNext = track.cameraAnimation[canIdx + 1];
            glm::vec3 vroadPoint = rotationMatrix * LibOpenNFS::Utils::FixedToFloat( LibOpenNFS::Utils::PointToVec(refPt.pt)) * NFS3::NFS3_SCALE_FACTOR;
            glm::vec3 vroadPointNext = rotationMatrix *  LibOpenNFS::Utils::FixedToFloat( LibOpenNFS::Utils::PointToVec(refPtNext.pt)) * NFS3::NFS3_SCALE_FACTOR;
            vroadPoint.y += 0.2f;
            vroadPointNext.y += 0.2f;
            m_bulletDebugDrawer->drawLine(
                    Utils::glmToBullet(vroadPoint + track.trackBlocks[0].position),
                    Utils::glmToBullet(vroadPointNext + track.trackBlocks[0].position), btVector3(0, 1, 1));

            // Draw Rotations
            glm::quat RotationMatrix =
                    glm::normalize(glm::quat(glm::vec3(glm::radians(0.f), glm::radians(-90.f), 0))) *
                    glm::normalize(glm::quat(refPt.od1 / 65536.0f, refPt.od2 / 65536.0f, refPt.od3 / 65536.0f, refPt.od4 / 65536.0f));
            glm::vec3 direction = glm::normalize(vroadPoint * glm::inverse(RotationMatrix));
            m_bulletDebugDrawer->drawLine(Utils::glmToBullet(vroadPoint + track.trackBlocks[0].position),
                                          Utils::glmToBullet(vroadPoint + track.trackBlocks[0].position + direction), btVector3(0, 0.5, 0.5));
        }
    }
} // namespace OpenNFS
