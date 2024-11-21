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
        void DrawTrackCollision(const std::shared_ptr<Track> &track);
        void DrawAABB(const AABB &aabb) const;
        void DrawFrustum(const BaseCamera &camera) const;
        void DrawCarRaycasts(const std::shared_ptr<Car> &car) const;
        void DrawVroad(const std::shared_ptr<Track> &track) const;
        void DrawCameraAnimation(const std::shared_ptr<Track> &track);

    private:
        std::shared_ptr<BulletDebugDrawer> m_bulletDebugDrawer;
    };
} // namespace OpenNFS
