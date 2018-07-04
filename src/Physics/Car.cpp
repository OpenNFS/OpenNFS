//
// Created by Amrik on 18/03/2018.
//


#include "Car.h"

Car::Car(NFS4_Loader loader){
    // Load these from Carp.txt
    gEngineForce = 0.f;
    gBreakingForce = 100.f;
    maxEngineForce = 2000.f;
    maxBreakingForce = 1000.f;
    suspensionRestLength = btScalar(0.064);
    suspensionStiffness = 500.f;
    suspensionDamping = 200.f;
    suspensionCompression = 200.4f;
    wheelFriction = 10000;
    rollInfluence = 0.04f;
    gVehicleSteering = 0.f;
    steeringIncrement = 0.01f;
    steeringClamp = 0.2f;
    steerRight = steerLeft = isReverse = false;

    glm::vec3 debug_offset(0, 0, 0);
    car_models = loader.meshes;

    // Enable High Res wheels and body
    for(int i = 0; i < 5; i++){
        car_models[i].enable();
        car_models[i].position += debug_offset;
    }

    glm::vec3 wheelDimensions = Utils::genDimensions(car_models[1].m_vertices);
    wheelRadius = wheelDimensions.z;
    wheelWidth = wheelDimensions.x;

    // the chassis collision shape
    btCollisionShape* chassisShape = Utils::genCollisionBox(car_models[0].m_vertices);
    m_collisionShapes.push_back(chassisShape);

    btCompoundShape* compound = new btCompoundShape();
    m_collisionShapes.push_back(compound);
    btTransform localTrans;
    localTrans.setIdentity();

    //Shift center of Mass (by 0 for now)
    localTrans.setOrigin(btVector3(0.0,0.0,0));
    compound->addChildShape(localTrans,chassisShape);

    float mass = 1000.0f;
    btVector3 localInertia(0,0,0);
    compound->calculateLocalInertia(mass,localInertia);

    // set initial location of vehicle in the world
    vehicleMotionState = new btDefaultMotionState(btTransform(
            btQuaternion(Utils::glmToBullet(car_models[0].orientation)),
            btVector3(Utils::glmToBullet(car_models[0].position))
    ));
    btRigidBody::btRigidBodyConstructionInfo cInfo(mass,vehicleMotionState,compound,localInertia);
    m_carChassis = new btRigidBody(cInfo);
    //m_carChassis->setDamping(0.2,0.2);

    m_carChassis -> setLinearVelocity(btVector3(0,0,0));
    m_carChassis -> setAngularVelocity(btVector3(0,0,0));
}

Car::~Car() {
    for (auto &car_model : car_models) {
        car_model.destroy();
    }

}

void Car::update() {
    btTransform trans;
    vehicleMotionState->getWorldTransform(trans);
    car_models[0].position = Utils::bulletToGlm(trans.getOrigin());
    car_models[0].orientation = Utils::bulletToGlm(trans.getRotation());
    car_models[0].update();

    for (int i = 0; i <m_vehicle->getNumWheels(); i++) {
        m_vehicle->updateWheelTransform(i, true);
        trans = m_vehicle->getWheelInfo(i).m_worldTransform;
        car_models[i + 1].position = Utils::bulletToGlm(trans.getOrigin());
        car_models[i + 1].orientation = Utils::bulletToGlm(trans.getRotation());
        car_models[i + 1].update();
    }

    // Set back wheels steering value
    int wheelIndex = 2;
    m_vehicle->applyEngineForce(gEngineForce,wheelIndex);
    m_vehicle->setBrake(gBreakingForce,wheelIndex);

    wheelIndex = 3;
    m_vehicle->applyEngineForce(gEngineForce,wheelIndex);
    m_vehicle->setBrake(gBreakingForce,wheelIndex);

    // update front wheels steering value
    if (steerRight)
    {
        gVehicleSteering -= steeringIncrement;
        if (gVehicleSteering < -steeringClamp) gVehicleSteering = -steeringClamp;
    }
    else if (steerLeft)
    {
        gVehicleSteering += steeringIncrement;
        if (gVehicleSteering > steeringClamp) gVehicleSteering = steeringClamp;
    }
    else
    {
      if(gVehicleSteering > 0){
          gVehicleSteering -= steeringIncrement;
      } else if (gVehicleSteering < 0){
          gVehicleSteering += steeringIncrement;
      }
    }

    // Set front wheels steering value
    wheelIndex = 0;
    m_vehicle->setSteeringValue(gVehicleSteering,wheelIndex);
    wheelIndex = 1;
    m_vehicle->setSteeringValue(gVehicleSteering,wheelIndex);
}

void Car::applyAccelerationForce(bool apply)
{
    if (apply) {
        if (!isReverse) gEngineForce = maxEngineForce;
        else gEngineForce = -maxEngineForce;
        gBreakingForce = 0.f;
    } else {
        gEngineForce = 0.f;
    }
}

void Car::applySteeringRight(bool apply)
{
    steerRight = apply;
}


void Car::applySteeringLeft(bool apply)
{
    steerLeft = apply;
}

void Car::applyBrakingForce(bool apply)
{
    if (apply) {
        gBreakingForce = maxBreakingForce;
    } else {
        gBreakingForce = 0.f;
    }
}

void Car::toggleReverse()
{
    isReverse = (isReverse? false : true);
}

void Car::resetCar()
{
    if (m_vehicle)
    {
        m_vehicle -> resetSuspension();
        for (int i = 0; i < m_vehicle->getNumWheels(); i++)
        {
            //synchronize the wheels with the (interpolated) chassis worldtransform
            m_vehicle -> updateWheelTransform(i, true);
        }
    }
}
