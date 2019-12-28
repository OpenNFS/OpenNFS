#pragma once

#include "BulletDebugDrawer.h"
#include "../Camera/Camera.h"
#include "../Loaders/TrackLoader.h"

class DebugRenderer
{
public:
    explicit DebugRenderer(std::shared_ptr<BulletDebugDrawer> bulletDebugDrawer);
    void Render(Camera &camera);
    void DrawTrackCollision(const std::shared_ptr<ONFSTrack> &track);
    void DrawAABB(const AABB &aabb);
    void DrawFrustum(Camera &camera);
    void DrawCarRaycasts(const std::shared_ptr<Car> &car);
    void DrawVroad(const std::shared_ptr<ONFSTrack> &track);
    void DrawCameraAnimation(Camera &camera, const std::shared_ptr<ONFSTrack> &track);

private:
    std::shared_ptr<BulletDebugDrawer> m_bulletDebugDrawer;
};
