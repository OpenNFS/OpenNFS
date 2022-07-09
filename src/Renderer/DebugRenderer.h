#pragma once

#include "BulletDebugDrawer.h"
#include "../Camera/BaseCamera.h"
#include "../Scene/Track.h"

class DebugRenderer {
public:
    explicit DebugRenderer(const std::shared_ptr<BulletDebugDrawer> &bulletDebugDrawer);
    void Render(const BaseCamera &camera);
    void DrawTrackCollision(const std::shared_ptr<Track> &track);
    void DrawAABB(const AABB &aabb);
    void DrawFrustum(const BaseCamera &camera);
    void DrawCarRaycasts(const std::shared_ptr<Car> &car);
    void DrawVroad(const std::shared_ptr<Track> &track);
    void DrawCameraAnimation(const std::shared_ptr<Track> &track);

private:
    std::shared_ptr<BulletDebugDrawer> m_bulletDebugDrawer;
};
