#pragma once

#include "../Camera/BaseCamera.h"
#include "../Physics/Car.h"
#include "../Scene/Track.h"
#include "BulletDebugDrawer.h"
#include "Text3DRenderer.h"

namespace OpenNFS {
    class RacerAgent;
    class DebugRenderer {
      public:
        explicit DebugRenderer(std::shared_ptr<BulletDebugDrawer> const &bulletDebugDrawer);
        void Render(BaseCamera const &camera) const;
        void DrawTrackCollision(std::shared_ptr<Track> const &track) const;
        void DrawAABB(const AABB &aabb, glm::vec3 const &colour) const;
        void DrawFrustum(BaseCamera const &camera) const;
        void DrawVehicleRaycasts(std::shared_ptr<Car> const &car) const;
        void DrawVehicleAI(std::shared_ptr<RacerAgent> const &racer, BaseCamera const &camera) const;
        void DrawDummy(glm::vec3 position, glm::vec3 direction) const;
        void DrawVroad(std::shared_ptr<Track> const &track) const;
        void DrawCameraAnimation(std::shared_ptr<Track> const &track) const;

      private:
        std::shared_ptr<BulletDebugDrawer> m_bulletDebugDrawer;
        Text3DRenderer m_text3DRenderer;
    };
} // namespace OpenNFS
