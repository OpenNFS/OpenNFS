#pragma once

#include "BulletDebugDrawer.h"
#include "../Camera/BaseCamera.h"
#include "../Scene/Track.h"
#include "../Physics/Car.h"

namespace OpenNFS {
    class DebugRenderer {
    public:
        explicit DebugRenderer(const std::shared_ptr<BulletDebugDrawer> &bulletDebugDrawer);
        void Render(const BaseCamera &camera) const;
        void DrawTrackCollision(const Track &track) const;
        void DrawMiniMap(BaseCamera const &camera, const Track &track) const;
        void DrawAABB(const AABB &aabb, glm::vec3 const &colour) const;
        void DrawFrustum(const BaseCamera &camera) const;
        void DrawCarRaycasts(const std::shared_ptr<Car> &car) const;
        void DrawDummy(glm::vec3 position, glm::vec3 direction) const;
        void DrawVroad(const Track &track) const;
        void DrawCameraAnimation(const Track &track) const;

    private:
        std::shared_ptr<BulletDebugDrawer> m_bulletDebugDrawer;
    };
} // namespace OpenNFS
