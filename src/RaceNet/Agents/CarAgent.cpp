#include "CarAgent.h"

namespace OpenNFS {
    CarAgent::CarAgent(AgentType agentType, const std::shared_ptr<Car> &car, const std::shared_ptr<Track> &track) :
        vehicle(std::make_shared<Car>(car->assetData, car->tag, car->id)), m_track(track), m_agentType(agentType) {
    }

    void CarAgent::ResetToIndexInTrackblock(int trackBlockIndex, int posIndex, float offset) {
        glm::vec3 vroadPoint;
        glm::quat carOrientation;

        ASSERT(offset <= 1.f, "Cannot reset to offset larger than +- 1.f on VROAD (Will spawn off track!)");

        // Can move this by trk[trackBlockIndex].nodePositions
        uint32_t nodeNumber = m_track->rawTrack->trackBlocks[trackBlockIndex].virtualRoadStartIndex;
        uint32_t nPositions = m_track->rawTrack->trackBlocks[trackBlockIndex].nVirtualRoadPositions;

        if (posIndex <= nPositions) {
            nodeNumber += posIndex;
        } else {
            // Advance the trackblock until we can get to posIndex
            int nExtra = posIndex - nPositions;
            while (true) {
                nodeNumber = m_track->rawTrack->trackBlocks[++trackBlockIndex].virtualRoadStartIndex;
                nPositions = m_track->rawTrack->trackBlocks[trackBlockIndex].nVirtualRoadPositions;
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
        ASSERT(offset <= 1.f, "Cannot reset to offset larger than +- 1.f on VROAD (Will spawn off track!)");
        ASSERT(vroadIndex < m_track->rawTrack->virtualRoad.size(), "Requested reset to vroad index: " << vroadIndex << " outside of num vroad chunks");

        glm::vec3 vroadPoint = m_track->rawTrack->virtualRoad[vroadIndex].position + m_track->rawTrack->virtualRoad[vroadIndex].respawn;
        glm::quat carOrientation =
          glm::conjugate(glm::toQuat(glm::lookAt(vroadPoint, vroadPoint - m_track->rawTrack->virtualRoad[vroadIndex].forward, m_track->rawTrack->virtualRoad[vroadIndex].normal)));
        // Offset horizontally across the right vector from center
        vroadPoint += offset * m_track->rawTrack->virtualRoad[vroadIndex].right;

        // Go and find the Vroad Data to reset to
        vehicle->SetPosition(vroadPoint, carOrientation);
    }

    void CarAgent::_UpdateNearestTrackblock() {
        float lowestDistance = FLT_MAX;

        // Get closest track block to car body position
        for (auto &trackblock : m_track->rawTrack->trackBlocks) {
            float distance = glm::distance(vehicle->carBodyModel.position, trackblock.position);
            if (distance < lowestDistance) {
                nearestTrackblockID = trackblock.id;
                lowestDistance      = distance;
            }
        }
    }

    void CarAgent::_UpdateNearestVroad() {
        float lowestDistance     = FLT_MAX;
        glm::quat rotationMatrix = glm::normalize(glm::quat(glm::vec3(-SIMD_PI / 2, 0, 0)));

        // Use the nearest trackblock ID to avoid skipping the entire set of Vroad data
        uint32_t nodeNumber = m_track->rawTrack->trackBlocks[nearestTrackblockID].virtualRoadStartIndex;
        uint32_t nPositions = m_track->rawTrack->trackBlocks[nearestTrackblockID].nVirtualRoadPositions;

        // Get closest vroad in trackblock set to car body position
        for (uint32_t vroadIdx = nodeNumber; vroadIdx < nodeNumber + nPositions; ++vroadIdx) {
            float distance = glm::distance(vehicle->carBodyModel.position, m_track->rawTrack->virtualRoad[vroadIdx].position);

            if (distance < lowestDistance) {
                m_nearestVroadID = vroadIdx;
                lowestDistance   = distance;
            }
        }
    }
} // namespace OpenNFS
