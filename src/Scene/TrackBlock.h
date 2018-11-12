//
// Created by Amrik Sadhra on 26/02/2018.
//

#pragma once

#include "Entity.h"
#include "Light.h"
#include "Sound.h"
#include "Track.h"
#include <BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h>
#include <BulletCollision/CollisionShapes/btTriangleMesh.h>

class TrackBlock {
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
