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

        DrawDummy(car->leftHeadLight.position, car->leftHeadLight.direction);
        DrawDummy(car->rightHeadLight.position, car->rightHeadLight.direction);
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
        constexpr float vRoadDisplayHeight = 0.2f;
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
            glm::vec3 vroadPoint{LibOpenNFS::Utils::FixedToFloat(refPt.pt) * NFS3::SCALE_FACTOR};
            glm::vec3 vroadPointNext{LibOpenNFS::Utils::FixedToFloat(refPtNext.pt) * NFS3::SCALE_FACTOR};
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

    void DebugRenderer::DrawNFS4PhysicsDebug(std::shared_ptr<Car> const &car) const {
        auto const *physics = car->GetNFS4VehiclePhysics();
        if (!physics) {
            return;
        }

        auto const &debug = physics->GetDebugData();
        auto const &state = physics->GetState();
        auto const &toggles = physics->GetToggles();
        btRigidBody const *chassis = physics->GetChassis();
        btTransform const trans = chassis->getWorldTransform();

        // Force visualization scale
        constexpr float kForceScale = 0.1f;
        constexpr float kVelocityScale = 0.05f;

        // Colors for different visualizations
        btVector3 const colorForce(1.0f, 0.5f, 0.0f);       // Orange - total force
        btVector3 const colorVelocity(0.0f, 1.0f, 1.0f);    // Cyan - velocity
        btVector3 const colorWheelForceX(1.0f, 0.0f, 0.0f); // Red - lateral wheel force
        btVector3 const colorWheelForceZ(0.0f, 1.0f, 0.0f); // Green - longitudinal wheel force
        btVector3 const colorSlip(1.0f, 1.0f, 0.0f);        // Yellow - slip indicator
        btVector3 const colorGrip(0.0f, 0.5f, 1.0f);        // Blue - grip indicator

        // Draw velocity vector from car center
        btVector3 const carPos = trans.getOrigin();
        btVector3 const worldVelocity = trans.getBasis() * debug.localVelocity;
        m_bulletDebugDrawer->drawLine(carPos, carPos + worldVelocity * kVelocityScale, colorVelocity);

        // Draw total force vector from car center
        btVector3 const worldForce = trans.getBasis() * debug.totalForce;
        m_bulletDebugDrawer->drawLine(carPos, carPos + worldForce * kForceScale, colorForce);

        // Draw per-wheel forces
        for (int i = 0; i < 4; i++) {
            btVector3 const wheelPos = debug.wheelWorldPositions[i];
            btVector3 const &localForce = debug.wheelForces[i];

            // Draw lateral (X) and longitudinal (Z) components separately
            btVector3 lateralWorld = trans.getBasis() * btVector3(localForce.x(), 0, 0);
            btVector3 longitudinalWorld = trans.getBasis() * btVector3(0, 0, localForce.z());

            m_bulletDebugDrawer->drawLine(wheelPos, wheelPos + lateralWorld * kForceScale, colorWheelForceX);
            m_bulletDebugDrawer->drawLine(wheelPos, wheelPos + longitudinalWorld * kForceScale, colorWheelForceZ);

            // Draw grip circle at each wheel (radius proportional to grip)
            float const grip = debug.wheels[i].grip;
            float const gripRadius = grip * 0.01f;

            // Draw a simple cross to represent grip magnitude
            //btVector3 const right = trans.getBasis().getColumn(0) * gripRadius;
            //btVector3 const forward = trans.getBasis().getColumn(2) * gripRadius;
            //m_bulletDebugDrawer->drawLine(wheelPos - right, wheelPos + right, colorGrip);
            //m_bulletDebugDrawer->drawLine(wheelPos - forward, wheelPos + forward, colorGrip);

            // Draw traction indicator (line proportional to traction)
            //float const traction = debug.wheels[i].traction;
            //btVector3 const tractionVec = trans.getBasis() * btVector3(0, 0, traction * 0.05f);
            //m_bulletDebugDrawer->drawLine(wheelPos + btVector3(0, 0.1f, 0), wheelPos + btVector3(0, 0.1f, 0) + tractionVec,
            //                              btVector3(0.5f, 0.5f, 1.0f));
        }

        // Draw slip angle indicator
        if (std::abs(state.slipAngle) > 0.05f) {
            // Draw a line showing slip direction
            float const slipMagnitude = std::min(std::abs(state.slipAngle) * 2.0f, 1.0f);
            btVector3 const slipDir = trans.getBasis() * btVector3(state.slipAngle > 0 ? 1 : -1, 0, 0);
            btVector3 const slipColor(slipMagnitude, 1.0f - slipMagnitude, 0);
            m_bulletDebugDrawer->drawLine(carPos + btVector3(0, 0.5f, 0), carPos + btVector3(0, 0.5f, 0) + slipDir * slipMagnitude,
                                          slipColor);
        }

        // Draw steering angle visualisation on front wheels
        float const steerAngle = state.steeringAngle * SIMD_2_PI;
        for (int i = 0; i < 2; i++) { // Front wheels only
            btVector3 const wheelPos = debug.wheelWorldPositions[i];

            // Draw a line showing wheel direction
            btVector3 localDir(std::sin(steerAngle), 0, std::cos(steerAngle));
            btVector3 const worldDir = trans.getBasis() * localDir * 0.3f;
            m_bulletDebugDrawer->drawLine(wheelPos, wheelPos + worldDir, btVector3(1, 1, 1));
        }

        // Draw ground orientation (car up vector vs world up)
        btVector3 const carUp = trans.getBasis().getColumn(1);
        m_bulletDebugDrawer->drawLine(carPos, carPos + carUp * 0.5f, btVector3(0.5f, 0.5f, 0.5f));

        // Draw handbrake indicator
        if (state.handbrakeInput) {
            // Draw red circles at rear wheels
            for (int i = 2; i < 4; i++) {
                btVector3 const wheelPos = debug.wheelWorldPositions[i];
                btVector3 const right = trans.getBasis().getColumn(0) * 0.02f;
                btVector3 const forward = trans.getBasis().getColumn(2) * 0.02f;
                m_bulletDebugDrawer->drawLine(wheelPos - right - forward, wheelPos + right - forward, btVector3(1, 0, 0));
                m_bulletDebugDrawer->drawLine(wheelPos + right - forward, wheelPos + right + forward, btVector3(1, 0, 0));
                m_bulletDebugDrawer->drawLine(wheelPos + right + forward, wheelPos - right + forward, btVector3(1, 0, 0));
                m_bulletDebugDrawer->drawLine(wheelPos - right + forward, wheelPos - right - forward, btVector3(1, 0, 0));
            }
        }

        // Draw lost grip indicator
        if (state.lostGrip) {
            // Draw yellow warning box above car
            btVector3 const warnPos = carPos + btVector3(0, 1.0f, 0);
            btVector3 const warnSize(0.2f, 0.1f, 0.2f);
            m_bulletDebugDrawer->drawBox(warnPos - warnSize, warnPos + warnSize, btVector3(1, 1, 0));
        }

        // Draw turning circle indicator (angular damping)
        //if (toggles.enableTurningCircle && std::abs(debug.turningCircleAngularDamp) > 0.01f) {
        //    // Draw arc showing turning circle effect
        //    btVector3 const turnColor(0.0f, 1.0f, 0.5f); // Teal
        //    float const turnMagnitude = std::min(std::abs(debug.turningCircleAngularDamp) * 0.5f, 1.0f);
        //    btVector3 const turnDir = trans.getBasis() * btVector3(debug.turningCircleAngularDamp > 0 ? 1 : -1, 0.5f, 0);
        //    m_bulletDebugDrawer->drawLine(carPos, carPos + turnDir * turnMagnitude, turnColor);
        //}

        // Draw lateral damping indicator
       //if (toggles.enableLateralDamping && std::abs(debug.lateralVelocityDamp) > 0.01f) {
       //    btVector3 const dampColor(0.5f, 0.0f, 1.0f); // Purple
       //    float const dampMagnitude = std::min(std::abs(debug.lateralVelocityDamp) * 0.1f, 0.5f);
       //    btVector3 const dampDir = trans.getBasis() * btVector3(debug.lateralVelocityDamp > 0 ? -1 : 1, 0, 0);
       //    m_bulletDebugDrawer->drawLine(carPos + btVector3(0, 0.3f, 0),
       //                                  carPos + btVector3(0, 0.3f, 0) + dampDir * dampMagnitude,
       //                                  dampColor);
       //}

       //// Draw near-stop deceleration indicator
       //if (toggles.enableNearStopDecel && debug.appliedNearStopDecel) {
       //    // Draw downward arrows at front and back to indicate stopping
       //    btVector3 const stopColor(1.0f, 0.5f, 0.0f); // Orange
       //    btVector3 const front = carPos + trans.getBasis() * btVector3(0, 0.2f, 1.0f);
       //    btVector3 const back = carPos + trans.getBasis() * btVector3(0, 0.2f, -1.0f);
       //    m_bulletDebugDrawer->drawLine(front, front - btVector3(0, 0.3f, 0), stopColor);
       //    m_bulletDebugDrawer->drawLine(back, back - btVector3(0, 0.3f, 0), stopColor);
       //}

       //// Draw neutral gear deceleration indicator
       //if (toggles.enableNeutralGearDecel && debug.appliedNeutralDecel) {
       //    // Draw 'N' shape at center of car
       //    btVector3 const neutralColor(0.7f, 0.7f, 0.7f); // Gray
       //    btVector3 const center = carPos + btVector3(0, 0.8f, 0);
       //    m_bulletDebugDrawer->drawLine(center + btVector3(-0.1f, -0.1f, 0), center + btVector3(-0.1f, 0.1f, 0), neutralColor);
       //    m_bulletDebugDrawer->drawLine(center + btVector3(-0.1f, 0.1f, 0), center + btVector3(0.1f, -0.1f, 0), neutralColor);
       //    m_bulletDebugDrawer->drawLine(center + btVector3(0.1f, -0.1f, 0), center + btVector3(0.1f, 0.1f, 0), neutralColor);
       //}

       //// Draw prevented sideways movement indicator
       //if (toggles.enablePreventSideways && debug.preventedSideways) {
       //    // Draw X at ground level to indicate locked position
       //    btVector3 const lockColor(1.0f, 0.0f, 1.0f); // Magenta
       //    btVector3 const groundPos = carPos - btVector3(0, 0.3f, 0);
       //    m_bulletDebugDrawer->drawLine(groundPos + btVector3(-0.2f, 0, -0.2f), groundPos + btVector3(0.2f, 0, 0.2f), lockColor);
       //    m_bulletDebugDrawer->drawLine(groundPos + btVector3(-0.2f, 0, 0.2f), groundPos + btVector3(0.2f, 0, -0.2f), lockColor);
       //}

       //// Draw airborne drag indicator
       //if (toggles.enableAirborneDrag && debug.airborneDownforce > 0.01f) {
       //    btVector3 const dragColor(0.3f, 0.3f, 1.0f); // Light blue
       //    float const dragMagnitude = std::min(debug.airborneDownforce * 0.5f, 1.0f);
       //    // Draw arrows pointing back/down to show air resistance
       //    btVector3 const dragDir = -chassis->getLinearVelocity().normalized() * dragMagnitude;
       //    m_bulletDebugDrawer->drawLine(carPos, carPos + dragDir, dragColor);
       //}

       //// Draw road adjustment angular velocity
       //if (toggles.enableAdjustToRoad && debug.roadAdjustmentAngVel.length() > 0.01f) {
       //    btVector3 const adjustColor(0.0f, 0.8f, 0.8f); // Cyan
       //    btVector3 const adjustDir = debug.roadAdjustmentAngVel.normalized() * 0.3f;
       //    m_bulletDebugDrawer->drawLine(carPos + btVector3(0, 0.6f, 0),
       //                                  carPos + btVector3(0, 0.6f, 0) + adjustDir,
       //                                  adjustColor);
       //}
    }
} // namespace OpenNFS
