#include "Track.h"

void Track::GenerateSpline()
{
    // Build a spline through the center of the track
    std::vector<glm::vec3> cameraPoints;
    for (auto &trackBlock : trackBlocks)
    {
        cameraPoints.emplace_back(glm::vec3(trackBlock.position.x, trackBlock.position.y + 0.2, trackBlock.position.z));
    }
    centerSpline = HermiteCurve(cameraPoints, 0.1f, 0.0f);
}

void Track::GenerateAabbTree()
{
    // Build an optimised BvH of AABB's so that culling for render becomes cheap
    for (auto &trackBlock : trackBlocks)
    {
        for (auto &baseTrackEntity : trackBlock.track)
        {
            cullTree.insertObject(std::make_shared<Entity>(baseTrackEntity));
        }
        for (auto &trackObjectEntity : trackBlock.objects)
        {
            cullTree.insertObject(std::make_shared<Entity>(trackObjectEntity));
        }
        for (auto &trackLaneEntity : trackBlock.lanes)
        {
            cullTree.insertObject(std::make_shared<Entity>(trackLaneEntity));
        }
    }
}