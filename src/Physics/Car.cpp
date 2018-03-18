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

    /*fr_TireBodyMotionstate = new btDefaultMotionState(btTransform(
            btQuaternion(Utils::glmToBullet(car_models[1].orientation)),
            btVector3(Utils::glmToBullet(car_models[1].position))
    ));
    fl_TireRigidBodyCI = btRigidBody::btRigidBodyConstructionInfo(
            1,                  // mass, in kg. 0 -> Static object, will never move.
            fl_TireBodyMotionstate,
            //genCollisionBox(car_models[1].m_vertices),  // collision shape of body
            new btBoxShape(btVector3(5,5,5)),
            btVector3(0, 0,0)    // local inertia
    );
    fl_TireRigidBody = new btRigidBody(fl_TireRigidBodyCI);
    fl_TireRigidBody->setUserPointer(this);*/

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

    /*fl_TireRigidBody->getMotionState()->getWorldTransform(trans);
    car_models[1].position = Utils::bulletToGlm(trans.getOrigin());
    car_models[1].orientation = Utils::bulletToGlm(trans.getRotation());
    car_models[1].update();*/
}
