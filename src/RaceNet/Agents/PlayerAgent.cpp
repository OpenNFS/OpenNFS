#include "PlayerAgent.h"

PlayerAgent::PlayerAgent(std::shared_ptr<Car> car, std::shared_ptr<ONFSTrack> raceTrack) :
CarAgent(AgentType::PLAYER, car, raceTrack)
{

}

void PlayerAgent::Simulate()
{

}