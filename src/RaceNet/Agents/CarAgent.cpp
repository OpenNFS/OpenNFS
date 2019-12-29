#include "CarAgent.h"

CarAgent::CarAgent(AgentType agentType, std::shared_ptr<Car> car, std::shared_ptr<ONFSTrack> track) :
vehicle(std::make_shared<Car>(car->assetData, car->tag, car->id)), m_track(track), m_agentType(agentType) { }

void CarAgent::ResetToIndexInTrackblock(int trackBlockIndex, int posIndex, float offset)
{
    glm::vec3 vroadPoint;
    glm::quat carOrientation;

    ASSERT(offset <= 1.f, "Cannot reset to offset larger than +- 1.f on VROAD (Will spawn off track!)");

    if (m_track->tag == NFS_3 || m_track->tag == NFS_4)
    {
        // Can move this by trk[trackBlockIndex].nodePositions
        uint32_t nodeNumber = boost::get<std::shared_ptr<NFS3_4_DATA::TRACK>>(m_track->trackData)->trk[trackBlockIndex].nStartPos;
        int nPositions = boost::get<std::shared_ptr<NFS3_4_DATA::TRACK>>(m_track->trackData)->trk[trackBlockIndex].nPositions;
        if (posIndex <= nPositions)
        {
            nodeNumber += posIndex;
        }
        else
        {
            // Advance the trackblock until we can get to posIndex
            int nExtra = posIndex - nPositions;
            while (true)
            {
                nodeNumber = boost::get<std::shared_ptr<NFS3_4_DATA::TRACK>>(m_track->trackData)->trk[++trackBlockIndex].nStartPos;
                nPositions = boost::get<std::shared_ptr<NFS3_4_DATA::TRACK>>(m_track->trackData)->trk[trackBlockIndex].nPositions;
                if (nExtra < nPositions)
                {
                    nodeNumber += nExtra;
                    break;
                }
                else
                {
                    nExtra -= nPositions;
                }
            }
        }
        this->ResetToVroad(nodeNumber, offset);
    }
    else
    {
        vroadPoint = Utils::PointToVec(m_track->trackBlocks[trackBlockIndex].center);
        vroadPoint.y += 0.2f;
        carOrientation = glm::quat(2, 0, 0, 1);

        // Go and find the Vroad Data to reset to
        vehicle->SetPosition(vroadPoint, carOrientation);
    }
}

void CarAgent::ResetToVroad(int vroadIndex, float offset)
{
    glm::vec3 vroadPoint;
    glm::quat carOrientation;

    ASSERT(offset <= 1.f, "Cannot reset to offset larger than +- 1.f on VROAD (Will spawn off track!)");

    if (m_track->tag == NFS_3 || m_track->tag == NFS_4)
    {
        uint32_t nVroad = boost::get<std::shared_ptr<NFS3_4_DATA::TRACK>>(m_track->trackData)->col.vroadHead.nrec;
        ASSERT(vroadIndex < nVroad, "Requested reset to vroad index: " << vroadIndex << " outside of nVroad: " << nVroad);

        glm::quat rotationMatrix = glm::normalize(glm::quat(glm::vec3(-SIMD_PI / 2, 0, 0)));
        COLVROAD resetVroad = boost::get<std::shared_ptr<NFS3_4_DATA::TRACK>>(m_track->trackData)->col.vroad[vroadIndex];
        vroadPoint = rotationMatrix * Utils::FixedToFloat(Utils::PointToVec(resetVroad.refPt)) / NFS3_SCALE_FACTOR;
        vroadPoint.y += 0.2f;

        // Get VROAD right vector
        glm::vec3 curVroadRightVec = rotationMatrix * Utils::PointToVec(resetVroad.right) / 128.f;
        vroadPoint += offset * curVroadRightVec;

        rotationMatrix = glm::normalize(glm::quat(glm::vec3(SIMD_PI / 2, 0, 0)));
        glm::vec3 forward = Utils::PointToVec(resetVroad.forward) * rotationMatrix;
        glm::vec3 normal = Utils::PointToVec(resetVroad.normal) * rotationMatrix;
        carOrientation = glm::conjugate(glm::toQuat(
                glm::lookAt(vroadPoint,
                            vroadPoint - forward,
                            normal
                )
        ));
    }
    else
    {
        ASSERT(false, "Vroad Index based reset not available outside NFS3 for now.");
    }

    // Go and find the Vroad Data to reset to
    vehicle->SetPosition(vroadPoint, carOrientation);
}

void CarAgent::_UpdateNearestTrackblock()
{
    float lowestDistance = FLT_MAX;

    // Get closest track block to car body position
    for (auto &trackblock :  m_track->trackBlocks)
    {
        float distance = glm::distance(vehicle->carBodyModel.position, trackblock.center);
        if (distance < lowestDistance)
        {
            nearestTrackblockID = trackblock.blockId;
            lowestDistance = distance;
        }
    }
}

void CarAgent::_UpdateNearestVroad()
{
    if (m_track->tag == NFS_3 || m_track->tag == NFS_4)
    {
        float lowestDistance = FLT_MAX;
        glm::quat rotationMatrix = glm::normalize(glm::quat(glm::vec3(-SIMD_PI / 2, 0, 0)));

        // Use the nearest trackblock ID to avoid skipping the entire set of Vroad data
        uint32_t nodeNumber = boost::get<std::shared_ptr<NFS3_4_DATA::TRACK>>(m_track->trackData)->trk[nearestTrackblockID].nStartPos;
        uint32_t nPositions = boost::get<std::shared_ptr<NFS3_4_DATA::TRACK>>(m_track->trackData)->trk[nearestTrackblockID].nPositions;

        // Get closest vroad in trackblock set to car body position
        for (uint32_t vroadIdx = nodeNumber; vroadIdx < nodeNumber + nPositions; ++vroadIdx)
        {
            COLVROAD resetVroad = boost::get<std::shared_ptr<NFS3_4_DATA::TRACK>>(m_track->trackData)->col.vroad[vroadIdx];
            glm::vec3 vroadPoint = rotationMatrix * (Utils::FixedToFloat(Utils::PointToVec(resetVroad.refPt)) / NFS3_SCALE_FACTOR);

            float distance = glm::distance(vehicle->carBodyModel.position, vroadPoint);
            if (distance < lowestDistance)
            {
                nearestTrackblockID = vroadIdx;
                lowestDistance = distance;
            }
        }
    }
}
