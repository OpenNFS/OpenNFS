//
// Created by Amrik on 18/03/2018.
//


#include <BulletCollision/CollisionShapes/btBoxShape.h>
#include "Car.h"
#include "../Util/Utils.h"


Car::Car(NFS_Loader loader){
    glm::vec3 debug_offset(90, 4, 0);
    car_models = loader.getMeshes();

    // Enable High Res wheels and body
    for(int i = 0; i < 5; i++){
        car_models[i].enable();
        car_models[i].position += debug_offset;
    }

    //Generate Physics collision data
    bodyMotionstate = new btDefaultMotionState(btTransform(
            btQuaternion(Utils::glmToBullet(car_models[0].orientation)),
            btVector3(Utils::glmToBullet(car_models[0].position))
    ));
    bodyRigidBodyCI = btRigidBody::btRigidBodyConstructionInfo(
            1625,                  // mass, in kg. 0 -> Static object, will never move.
            bodyMotionstate,
            Utils::genCollisionBox(car_models[0].m_vertices),  // collision shape of body
            btVector3(0, 0,0)    // local inertia
    );
    bodyRigidBody = new btRigidBody(bodyRigidBodyCI);
    bodyRigidBody->setUserPointer(this);

    fl_TireBodyMotionstate = new btDefaultMotionState(btTransform(
            btQuaternion(Utils::glmToBullet(car_models[1].orientation)),
            btVector3(Utils::glmToBullet(car_models[1].position))
    ));
    fl_TireRigidBodyCI = btRigidBody::btRigidBodyConstructionInfo(
            1,                  // mass, in kg. 0 -> Static object, will never move.
            fl_TireBodyMotionstate,
            Utils::genCollisionBox(car_models[1].m_vertices),  // collision shape of body
            btVector3(0, 0,0)    // local inertia
    );
    fl_TireRigidBody = new btRigidBody(fl_TireRigidBodyCI);
    fl_TireRigidBody->setUserPointer(this);



    fr_TireBodyMotionstate = new btDefaultMotionState(btTransform(
            btQuaternion(Utils::glmToBullet(car_models[2].orientation)),
            btVector3(Utils::glmToBullet(car_models[2].position))
    ));
    fr_TireRigidBodyCI = btRigidBody::btRigidBodyConstructionInfo(
            1,                  // mass, in kg. 0 -> Static object, will never move.
            fr_TireBodyMotionstate,
            Utils::genCollisionBox(car_models[2].m_vertices),  // collision shape of body
            btVector3(0, 0,0)    // local inertia
    );
    fr_TireRigidBody = new btRigidBody(fr_TireRigidBodyCI);
    fr_TireRigidBody->setUserPointer(this);



    bl_TireBodyMotionstate = new btDefaultMotionState(btTransform(
            btQuaternion(Utils::glmToBullet(car_models[3].orientation)),
            btVector3(Utils::glmToBullet(car_models[3].position))
    ));
    bl_TireRigidBodyCI = btRigidBody::btRigidBodyConstructionInfo(
            1,                  // mass, in kg. 0 -> Static object, will never move.
            bl_TireBodyMotionstate,
            Utils::genCollisionBox(car_models[3].m_vertices),  // collision shape of body
            btVector3(0, 0,0)    // local inertia
    );
    bl_TireRigidBody = new btRigidBody(bl_TireRigidBodyCI);
    bl_TireRigidBody->setUserPointer(this);



    br_TireBodyMotionstate = new btDefaultMotionState(btTransform(
            btQuaternion(Utils::glmToBullet(car_models[4].orientation)),
            btVector3(Utils::glmToBullet(car_models[4].position))
    ));
    br_TireRigidBodyCI = btRigidBody::btRigidBodyConstructionInfo(
            1,                  // mass, in kg. 0 -> Static object, will never move.
            br_TireBodyMotionstate,
            Utils::genCollisionBox(car_models[4].m_vertices),  // collision shape of body
            btVector3(0, 0,0)    // local inertia
    );
    br_TireRigidBody = new btRigidBody(br_TireRigidBodyCI);
    br_TireRigidBody->setUserPointer(this);
}

Car::~Car() {
    for (auto &car_model : car_models) {
        car_model.destroy();
    }
}

void Car::update() {
    btTransform trans;
    bodyRigidBody->getMotionState()->getWorldTransform(trans);
    car_models[0].position = Utils::bulletToGlm(trans.getOrigin());
    car_models[0].orientation = Utils::bulletToGlm(trans.getRotation());
    car_models[0].update();

    fl_TireRigidBody->getMotionState()->getWorldTransform(trans);
    car_models[1].position = Utils::bulletToGlm(trans.getOrigin());
    car_models[1].orientation = Utils::bulletToGlm(trans.getRotation());
    car_models[1].update();

    fr_TireRigidBody->getMotionState()->getWorldTransform(trans);
    car_models[2].position = Utils::bulletToGlm(trans.getOrigin());
    car_models[2].orientation = Utils::bulletToGlm(trans.getRotation());
    car_models[2].update();

    bl_TireRigidBody->getMotionState()->getWorldTransform(trans);
    car_models[3].position = Utils::bulletToGlm(trans.getOrigin());
    car_models[3].orientation = Utils::bulletToGlm(trans.getRotation());
    car_models[3].update();

    br_TireRigidBody->getMotionState()->getWorldTransform(trans);
    car_models[4].position = Utils::bulletToGlm(trans.getOrigin());
    car_models[4].orientation = Utils::bulletToGlm(trans.getRotation());
    car_models[4].update();
}
