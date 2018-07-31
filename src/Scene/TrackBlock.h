//
// Created by Amrik on 26/02/2018.
//

#pragma once

#include <BulletCollision/CollisionShapes/btTriangleMesh.h>
#include "Track.h"
#include "Light.h"
#include "Sound.h"
#include "BulletPtr.h"

class TrackBlock  : public BulletPtr {
public:
    TrackBlock(int blockID, glm::vec3 center_pos);
    void generatePhysicsMesh();

    glm::vec3 center;
    int block_id;
    std::vector<Track> track;
    std::vector<Track> objects;
    std::vector<Track> lanes;
    std::vector<Light> lights;
    std::vector<Sound> sounds;

    // Physics
    btTriangleMesh trackMesh;
    btRigidBody* trackRigidBody;
    btCollisionShape* trackShape;
    btDefaultMotionState* groundMotionState;
};
