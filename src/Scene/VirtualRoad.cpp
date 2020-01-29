#include "VirtualRoad.h"

VirtualRoad::VirtualRoad(glm::vec3 position, uint32_t unknown, glm::vec3 normal, glm::vec3 forward, glm::vec3 right, glm::vec3 leftWall, glm::vec3 rightWall)
{
	this->position  = position;
	this->unknown   = unknown;
	this->normal    = normal;
	this->forward   = forward;
	this->right     = right;
	this->leftWall  = leftWall;
	this->rightWall = rightWall;
}
