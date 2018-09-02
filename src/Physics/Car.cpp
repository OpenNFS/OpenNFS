//
// Created by Amrik Sadhra on 18/03/2018.
//


#include "Car.h"
#include "../Scene/Entity.h"

Car::Car(std::vector<CarModel> car_meshes, NFSVer nfs_version, std::string car_name, std::map<unsigned int, GLuint> texture_gl_mappings) : Car(car_meshes, nfs_version, car_name) {
    car_texture_gl_mappings = texture_gl_mappings;
    multitexturedCarModel = true;
}


Car::Car(std::vector<CarModel> car_meshes, NFSVer nfs_version, std::string car_name){
    tag = nfs_version;
    name = car_name;

    // Load these from Carp.txt
    gEngineForce = 0.f;
    gBreakingForce = 100.f;
    maxEngineForce = 4000.f;
    maxBreakingForce = 1000.f;
    suspensionRestLength = btScalar(0.026);
    suspensionStiffness = 500.f;
    suspensionDamping = 200.f;
    suspensionCompression = 500.4f;
    wheelFriction = 100000;
    rollInfluence = 0.04f;
    gVehicleSteering = 0.f;
    steeringIncrement = 0.01f;
    steeringClamp = 0.2f;
    steerRight = steerLeft = false;

    setModels(car_meshes);

    glm::vec3 wheelDimensions = Utils::genDimensions(left_front_wheel_model.m_vertices);
    wheelRadius = wheelDimensions.z;
    wheelWidth = wheelDimensions.x;

    // the chassis collision shape
    btCollisionShape* chassisShape = Utils::genCollisionBox(car_body_model.m_vertices);
    m_collisionShapes.push_back(chassisShape);

    btCompoundShape* compound = new btCompoundShape();
    m_collisionShapes.push_back(compound);
    btTransform localTrans;
    localTrans.setIdentity();

    //Shift center of Mass (by 0 for now)
    localTrans.setOrigin(btVector3(0.0,0.0,0));
    compound->addChildShape(localTrans,chassisShape);

    float mass = 2000.0f;
    btVector3 localInertia(0,0,0);
    compound->calculateLocalInertia(mass,localInertia);

    // set initial location of vehicle in the world
    vehicleMotionState = new btDefaultMotionState(btTransform(btQuaternion(Utils::glmToBullet(car_body_model.orientation)), btVector3(Utils::glmToBullet(car_body_model.position))));
    btRigidBody::btRigidBodyConstructionInfo cInfo(mass,vehicleMotionState,compound,localInertia);
    m_carChassis = new btRigidBody(cInfo);
    // Abuse Entity system with a dummy entity that wraps the car pointer instead of a GL mesh
    m_carChassis->setUserPointer(new Entity(-1, -1, tag, EntityType::CAR, this));
    m_carChassis->setDamping(0.2,0.2);
    m_carChassis -> setLinearVelocity(btVector3(0,0,0));
    m_carChassis -> setAngularVelocity(btVector3(0,0,0));
}

// Take the list of Meshes returned by the car loader, and pull the High res wheels and body out for physics to manipulate
void Car::setModels(std::vector<CarModel> loaded_car_models){
    uint8_t usedModelCount = 0;

    switch(tag){
        case NFS_1:break;
        case NFS_2_PS1:break;
        case NFS_2_SE:
        case NFS_2:
        case NFS_3_PS1:
            if(loaded_car_models.size() < 3){
                loaded_car_models[0].enable();
                left_front_wheel_model = loaded_car_models[0];
                right_front_wheel_model = loaded_car_models[0];
                left_rear_wheel_model = loaded_car_models[0];
                right_rear_wheel_model = loaded_car_models[0];
                loaded_car_models[1].enable();
                car_body_model = loaded_car_models[1];
            }else if(loaded_car_models.size() < 20){
                loaded_car_models[2].enable();
                left_front_wheel_model = loaded_car_models[2];
                right_front_wheel_model = loaded_car_models[2];
                left_rear_wheel_model = loaded_car_models[2];
                right_rear_wheel_model = loaded_car_models[2];
                for(auto& car_model : loaded_car_models) {
                    if (car_model.m_name == "Medium Main Body Part") {
                        car_model.enable();
                        car_body_model = car_model;
                    } else if (car_model.m_name.find("Medium") != std::string::npos && car_model.m_name.find("Wheel") == std::string::npos) {
                        if(car_model.m_name != loaded_car_models[2].m_name){
                            car_model.enable();
                            misc_models.emplace_back(car_model);
                        }
                    } else {
                        if(car_model.m_name != loaded_car_models[2].m_name){
                            misc_models.emplace_back(car_model);
                        }
                    }
                }
            } else {
                for(auto& car_model : loaded_car_models){
                    if(car_model.m_name == "High Main Body Part"){
                        car_model.enable();
                        car_body_model = car_model;
                    } else if(car_model.m_name.find("High Front Left Wheel Part") != std::string::npos){
                        car_model.enable();
                        left_front_wheel_model = car_model;
                    } else if(car_model.m_name.find("High Front Right Wheel Part") != std::string::npos){
                        car_model.enable();
                        right_front_wheel_model = car_model;
                    } else if(car_model.m_name.find("High Rear Left Wheel Part") != std::string::npos){
                        car_model.enable();
                        left_rear_wheel_model = car_model;
                    } else if(car_model.m_name.find("High Rear Right Wheel Part") != std::string::npos){
                        car_model.enable();
                        right_rear_wheel_model = car_model;
                    } else if(car_model.m_name.find("High") != std::string::npos){ // Everything with "High" in the name is an extra body part, enable it
                        car_model.enable();
                        misc_models.emplace_back(car_model);
                    } else {
                        misc_models.emplace_back(car_model);
                    }
                }
            }
            break;
        case NFS_3:
            if(loaded_car_models.size() < 5) {
                car_body_model = loaded_car_models[0];
                car_body_model.enable();
                for(auto& car_model : loaded_car_models){
                    if(car_model.m_name.find("medium l front wheel") != std::string::npos){
                        car_model.enable();
                        left_front_wheel_model = car_model;
                    } else if(car_model.m_name.find("medium r front wheel") != std::string::npos){
                        car_model.enable();
                        right_front_wheel_model = car_model;
                    } else if(car_model.m_name.find("medium l rear wheel") != std::string::npos){
                        car_model.enable();
                        left_rear_wheel_model = car_model;
                    } else if(car_model.m_name.find("medium r rear wheel") != std::string::npos){
                        car_model.enable();
                        right_rear_wheel_model = car_model;
                    } else {
                        if(car_model.m_name != loaded_car_models[0].m_name){
                            misc_models.emplace_back(car_model);
                        }
                    }
                }
            } else {
                for(auto& car_model : loaded_car_models){
                    if(car_model.m_name == "high body"){
                        car_model.enable();
                        car_body_model = car_model;
                    } else if(car_model.m_name.find("left front wheel") != std::string::npos){
                        car_model.enable();
                        left_front_wheel_model = car_model;
                    } else if(car_model.m_name.find("right front wheel") != std::string::npos){
                        car_model.enable();
                        right_front_wheel_model = car_model;
                    } else if(car_model.m_name.find("left rear wheel") != std::string::npos){
                        car_model.enable();
                        left_rear_wheel_model = car_model;
                    } else if(car_model.m_name.find("right rear wheel") != std::string::npos){
                        car_model.enable();
                        right_rear_wheel_model = car_model;
                    } else {
                        misc_models.emplace_back(car_model);
                    }
                }
            }
            break;
        case NFS_4:
            for(auto& car_model : loaded_car_models){
                if(car_model.m_name == ":HB"){
                    car_model.enable();
                    car_body_model = car_model;
                } else if(car_model.m_name == ":HLRW"){
                    car_model.enable();
                    left_rear_wheel_model = car_model;
                } else if(car_model.m_name == ":HLFW"){
                    car_model.enable();
                    left_front_wheel_model = car_model;
                } else if(car_model.m_name == ":HRRW"){
                    car_model.enable();
                    right_rear_wheel_model = car_model;
                } else if (car_model.m_name == ":HRFW"){
                    car_model.enable();
                    right_front_wheel_model = car_model;
                } else if(car_model.m_name.find("O") != std::string::npos) {
                    car_model.enable();
                    misc_models.emplace_back(car_model);
                } else{
                    misc_models.emplace_back(car_model);
                }
            }
            break;
        case UNKNOWN:
            break;
        case NFS_5:
            break;
    }
}

void Car::setPosition(glm::vec3 position){
    btTransform initialTransform;
    initialTransform.setOrigin(Utils::glmToBullet(position));
    initialTransform.setRotation(btQuaternion(0,0,0,1));
    m_carChassis->setWorldTransform(initialTransform);
    update();
}

Car::~Car() {
    left_front_wheel_model.destroy();
    right_front_wheel_model.destroy();
    left_rear_wheel_model.destroy();
    right_rear_wheel_model.destroy();
    car_body_model.destroy();
    for (auto &car_model : misc_models) {
        car_model.destroy();
    }
}

void Car::update() {
    btTransform trans;
    vehicleMotionState->getWorldTransform(trans);
    car_body_model.position = Utils::bulletToGlm(trans.getOrigin()) + (car_body_model.initialPosition * glm::inverse(Utils::bulletToGlm(trans.getRotation())));
    car_body_model.orientation = Utils::bulletToGlm(trans.getRotation());
    car_body_model.update();

    // Might as well apply the body transform to the Miscellaneous models
    for(auto &misc_model : misc_models){
        misc_model.position = Utils::bulletToGlm(trans.getOrigin()) + (misc_model.initialPosition * glm::inverse(Utils::bulletToGlm(trans.getRotation())));
        misc_model.orientation = Utils::bulletToGlm(trans.getRotation());
        misc_model.update();
    }

    for (int i = 0; i <m_vehicle->getNumWheels(); i++) {
        m_vehicle->updateWheelTransform(i, true);
        trans = m_vehicle->getWheelInfo(i).m_worldTransform;
        switch(i){
            case 0:
                left_front_wheel_model.position = Utils::bulletToGlm(trans.getOrigin());
                left_front_wheel_model.orientation = Utils::bulletToGlm(trans.getRotation());
                left_front_wheel_model.update();
                break;
            case 1:
                right_front_wheel_model.position = Utils::bulletToGlm(trans.getOrigin());
                right_front_wheel_model.orientation = Utils::bulletToGlm(trans.getRotation());
                right_front_wheel_model.update();
                break;
            case 2:
                left_rear_wheel_model.position = Utils::bulletToGlm(trans.getOrigin());
                left_rear_wheel_model.orientation = Utils::bulletToGlm(trans.getRotation());
                left_rear_wheel_model.update();
                break;
            case 3:
                right_rear_wheel_model.position = Utils::bulletToGlm(trans.getOrigin());
                right_rear_wheel_model.orientation = Utils::bulletToGlm(trans.getRotation());
                right_rear_wheel_model.update();
                break;
            default:
                ASSERT(false, "More than 4 wheels currently unsupported");
                break;
        }
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

void Car::applyAccelerationForce(bool accelerate, bool reverse)
{
    if (accelerate) {
        gEngineForce = maxEngineForce;
        gBreakingForce = 0.f;
    } else if (reverse) {
        gEngineForce = -maxEngineForce;
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


void Car::resetCar(glm::vec3 reset_position)
{
    setPosition(reset_position);
    if (m_vehicle)
    {
        m_carChassis->clearForces();
        m_carChassis->setLinearVelocity(btVector3(0,0,0));
        m_carChassis->setAngularVelocity(btVector3(0,0,0));
        m_vehicle -> resetSuspension();
        for (int i = 0; i < m_vehicle->getNumWheels(); i++)
        {
            //synchronize the wheels with the (interpolated) chassis worldtransform
            m_vehicle -> updateWheelTransform(i, true);
        }
    }
}


void Car::writeObj(const std::string &path) {
    std::cout << "Writing Meshes to " << path << std::endl;

    std::ofstream obj_dump;
    obj_dump.open(path);

    /*for (Model &mesh : car_body_model) {
        *//* Print Part name*//*
        obj_dump << "o " << mesh.m_name << std::endl;
        //Dump Vertices
        for (auto vertex : mesh.m_vertices) {
            obj_dump << "v " << vertex[0] << " " << vertex[1] << " " << vertex[2] << std::endl;
        }
        //Dump UVs
        for (auto uv : mesh.m_uvs) {
            obj_dump << "vt " << uv[0] << " " << uv[1] << std::endl;
        }
        //Dump Indices
        for (auto vert_index : mesh.m_vertex_indices) {
            obj_dump << "f " << vert_index << std::endl;
        }
    }*/

    obj_dump.close();
}

double Car::getRotY() {
    glm::quat orientation = car_body_model.orientation;
    return atan2(2*orientation.y*orientation.w - 2*orientation.x*orientation.z, 1 - 2*orientation.y*orientation.y - 2*orientation.z*orientation.z)* (180 / M_PI);
}
