//
// Created by Amrik on 18/03/2018.
//

#ifndef OPENNFS3_CAR_H
#define OPENNFS3_CAR_H

#include <LinearMath/btDefaultMotionState.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#include "../Loaders/nfs_loader.h"
#include "../Scene/CarModel.h"

class Car {
public:
    Car(NFS_Loader loader);
    ~Car();
    void update();
    std::vector<CarModel> car_models;
    // Physics
    // BODY
    btRigidBody *bodyRigidBody;
    btDefaultMotionState* bodyMotionstate;
    btRigidBody::btRigidBodyConstructionInfo bodyRigidBodyCI = btRigidBody::btRigidBodyConstructionInfo(0, nullptr, nullptr);
    // FL Tire
    btRigidBody *fl_TireRigidBody;
    btDefaultMotionState* fl_TireBodyMotionstate;
    btRigidBody::btRigidBodyConstructionInfo fl_TireRigidBodyCI = btRigidBody::btRigidBodyConstructionInfo(0, nullptr, nullptr);
    // FR Tire
    btRigidBody *fr_TireRigidBody;
    btDefaultMotionState* fr_TireBodyMotionstate;
    btRigidBody::btRigidBodyConstructionInfo fr_TireRigidBodyCI = btRigidBody::btRigidBodyConstructionInfo(0, nullptr, nullptr);
    // BL Tire
    btRigidBody *bl_TireRigidBody;
    btDefaultMotionState* bl_TireBodyMotionstate;
    btRigidBody::btRigidBodyConstructionInfo bl_TireRigidBodyCI = btRigidBody::btRigidBodyConstructionInfo(0, nullptr, nullptr);
    // BR Tire
    btRigidBody *br_TireRigidBody;
    btDefaultMotionState* br_TireBodyMotionstate;
    btRigidBody::btRigidBodyConstructionInfo br_TireRigidBodyCI = btRigidBody::btRigidBodyConstructionInfo(0, nullptr, nullptr);

private:

};


#endif //OPENNFS3_CAR_H
