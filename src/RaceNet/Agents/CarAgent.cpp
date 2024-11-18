#include "CarAgent.h"

namespace OpenNFS {
    CarAgent::CarAgent(AgentType agentType, const std::shared_ptr<Car> &car, const std::shared_ptr<Track> &track) : vehicle(car), m_track(track), m_agentType(agentType) {
    }

    void CarAgent::ResetToIndexInTrackblock(int trackBlockIndex, int posIndex, float offset) {
        glm::vec3 vroadPoint;
        glm::quat carOrientation;

        CHECK_F(offset <= 1.f, "Cannot reset to offset larger than +- 1.f on VROAD (Will spawn off track!)");

        // Can move this by trk[trackBlockIndex].nodePositions
        uint32_t nodeNumber = m_track->trackBlocks[trackBlockIndex].virtualRoadStartIndex;
        uint32_t nPositions = m_track->trackBlocks[trackBlockIndex].nVirtualRoadPositions;

        if (posIndex <= nPositions) {
            nodeNumber += posIndex;
        } else {
            // Advance the trackblock until we can get to posIndex
            int nExtra = posIndex - nPositions;
            while (true) {
                nodeNumber = m_track->trackBlocks[++trackBlockIndex].virtualRoadStartIndex;
                nPositions = m_track->trackBlocks[trackBlockIndex].nVirtualRoadPositions;
                if (nExtra < nPositions) {
                    nodeNumber += nExtra;
                    break;
                } else {
                    nExtra -= nPositions;
                }
            }
        }
        this->ResetToVroad(nodeNumber, offset);
    }

    void CarAgent::ResetToVroad(int vroadIndex, float offset) {
        CHECK_F(offset <= 1.f, "Cannot reset to offset larger than +- 1.f on VROAD (Will spawn off track!)");
        CHECK_F(vroadIndex < m_track->virtualRoad.size(), "Requested reset to vroad index: %d outside of num vroad chunks", vroadIndex);

        glm::vec3 vroadPoint = m_track->virtualRoad[vroadIndex].position + m_track->virtualRoad[vroadIndex].respawn;
        glm::quat carOrientation =
          glm::conjugate(glm::toQuat(glm::lookAt(vroadPoint, vroadPoint - m_track->virtualRoad[vroadIndex].forward, m_track->virtualRoad[vroadIndex].normal)));
        // Offset horizontally across the right vector from center
        vroadPoint += offset * m_track->virtualRoad[vroadIndex].right;

        // Go and find the Vroad Data to reset to
        vehicle->SetPosition(vroadPoint, carOrientation);
    }

    void CarAgent::_UpdateNearestTrackblock() {
        float lowestDistance = FLT_MAX;

        // Get closest track block to car body position
        for (auto &trackblock : m_track->trackBlocks) {
            float distance = glm::distance(vehicle->carBodyModel.geometry->position, trackblock.position);
            if (distance < lowestDistance) {
                nearestTrackblockID = trackblock.id;
                lowestDistance      = distance;
            }
        }
    }

    void CarAgent::_UpdateNearestVroad() {
        float lowestDistance     = FLT_MAX;
        glm::quat rotationMatrix = glm::normalize(glm::quat(glm::vec3(-glm::pi<float>() / 2, 0, 0)));

        // Use the nearest trackblock ID to avoid skipping the entire set of Vroad data
        uint32_t nodeNumber = m_track->trackBlocks[nearestTrackblockID].virtualRoadStartIndex;
        uint32_t nPositions = m_track->trackBlocks[nearestTrackblockID].nVirtualRoadPositions;

        // Get closest vroad in trackblock set to car body position
        for (uint32_t vroadIdx = nodeNumber; vroadIdx < nodeNumber + nPositions; ++vroadIdx) {
            float distance = glm::distance(vehicle->carBodyModel.geometry->position, m_track->virtualRoad[vroadIdx].position);

            if (distance < lowestDistance) {
                m_nearestVroadID = vroadIdx;
                lowestDistance   = distance;
            }
        }
    }
} // namespace OpenNFS
