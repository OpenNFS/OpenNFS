#pragma once

#include "../RaceNet.h"
#include "../RaceNEAT.h"
#include "../../Loaders/CarLoader.h"
#include "../../Loaders/TrackLoader.h"

enum AgentType : uint8_t
{
    TRAINING = 0,
    RACING,
    PLAYER,
};

class CarAgent
{
public:
    void ResetToIndexInTrackblock(int trackBlockIndex, int posIndex, float offset);
    void ResetToVroad(int vroadIndex, float offset);
    virtual void Simulate() = 0;

    std::string name;
    std::shared_ptr<Car> vehicle;
    RaceNet raceNet;
    uint32_t nearestTrackblockID = 0;

protected:
    CarAgent(AgentType agentType, std::shared_ptr<Car> car, std::shared_ptr<ONFSTrack> track);
    void _UpdateNearestTrackblock();
    void _UpdateNearestVroad();

    std::shared_ptr<ONFSTrack> m_track;
    AgentType m_agentType;
    uint32_t m_nearestVroadID = 0;
};