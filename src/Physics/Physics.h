//
// Created by Amrik Sadhra on 05/02/2018.
//

#pragma once

#include <glm/vec3.hpp>
#include <BulletCollision/CollisionShapes/btBoxShape.h>
#include <vector>

btBoxShape* genCollisionBox(std::vector<glm::vec3> model_vertices);
