#pragma once

#include "../Camera/BaseCamera.h"
#include "../Physics/Car.h"
#include "../Scene/Track.h"
#include "BulletDebugDrawer.h"

namespace OpenNFS {
    class DebugRenderer {
      public:
        explicit DebugRenderer(std::shared_ptr<BulletDebugDrawer> const &bulletDebugDrawer);
        void Render(BaseCamera const &camera) const;
        void DrawTrackCollision(Track const &track) const;
        void DrawAABB(const AABB &aabb, glm::vec3 const &colour) const;
        void DrawFrustum(BaseCamera const &camera) const;
        void DrawCarRaycasts(std::shared_ptr<Car> const &car) const;
        void DrawDummy(glm::vec3 position, glm::vec3 direction) const;
        void DrawVroad(Track const &track) const;
        void DrawCameraAnimation(Track const &track) const;

      private:
        std::shared_ptr<BulletDebugDrawer> m_bulletDebugDrawer;
    };
} // namespace OpenNFS
