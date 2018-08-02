//
// Created by Amrik on 26/02/2018.
//

#pragma once

#include <BulletCollision/CollisionShapes/btTriangleMesh.h>
#include <BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h>
#include "Track.h"
#include "Light.h"
#include "Sound.h"
#include "Entity.h"

class TrackBlock  {
public:
    TrackBlock(int blockID, glm::vec3 center_pos);

    glm::vec3 center;
    int block_id;

    std::vector<Entity> track;
    std::vector<Entity> objects;
    std::vector<Entity> lanes;
    std::vector<Entity> lights;
    std::vector<Entity> sounds;
};
