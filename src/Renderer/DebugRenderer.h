#pragma once

#include "BulletDebugDrawer.h"
#include "../Camera/Camera.h"
#include "../Loaders/TrackLoader.h"

class DebugRenderer
{
public:
    explicit DebugRenderer(std::shared_ptr<BulletDebugDrawer> bulletDebugDrawer);
    void Render(const std::shared_ptr<Camera> &camera);
    void DrawTrackCollision(const std::shared_ptr<ONFSTrack> &track);
    void DrawAABB(const AABB &aabb);
    void DrawFrustum(const std::shared_ptr<Camera> &camera);
    void DrawCarRaycasts(const std::shared_ptr<Car> &car);
    void DrawVroad(const std::shared_ptr<ONFSTrack> &track);
    void DrawCameraAnimation(const std::shared_ptr<ONFSTrack> &track);

private:
    std::shared_ptr<BulletDebugDrawer> m_bulletDebugDrawer;
};
