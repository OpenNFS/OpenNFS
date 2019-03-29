//
// Created by Amrik Sadhra on 18/03/2018.
//


#include "Car.h"
#include "../Scene/Entity.h"

// TODO: Put this somewhere nice
// Forward casts should extend further than L/R
constexpr static const float kCastDistances[Car::kNumRangefinders] = {
        1.f,
        1.f,
        1.f,
        1.f,
        1.f,
        1.5f,
        2.f,
        3.f,
        5.f,
        5.f,
        5.f,
        3.f,
        2.f,
        2.f,
        1.5f,
        1.f,
        1.f,
        1.f,
        1.f,
};

Car::Car(CarData carData, NFSVer nfs_version, std::string carID, GLuint car_textureArrayID) : Car(carData, nfs_version, carID) {
    textureArrayID = car_textureArrayID;
    multitexturedCarModel = true;
}

Car::Car(CarData carData, NFSVer nfs_version, std::string carID) : id(carID), data(carData) {
    tag = nfs_version;
    if(tag == NFS_3 || tag == NFS_4){
        name = carData.carName;
    }

    if(!Config::get().vulkanRender){
        std::stringstream car_texture_path;
        car_texture_path << CAR_PATH << ToString(tag) << "/" << id << "/car00.tga";
        int width, height;
        textureID = ImageLoader::LoadImage(car_texture_path.str(), &width, &height, GL_CLAMP_TO_BORDER, GL_LINEAR_MIPMAP_LINEAR);
    }

    // Load these from Carp.txt
    maxSpeed = 20.f;
    gEngineForce = 0.f;
    gBreakingForce = 100.f;
    maxEngineForce = 3000.f;
    maxBreakingForce = 1000.f;
    suspensionRestLength = btScalar(0.020);
    suspensionStiffness = 750.f;
    suspensionDamping = 200.f;
    suspensionCompression = 500.4f;
    wheelFriction = 0.45f;
    rollInfluence = 0.04f;
    gVehicleSteering = 0.f;
    steeringIncrement = 0.01f;
    steeringClamp = 0.15f;
    steerRight = steerLeft = false;

    // Map mesh data to car data
    setModels(data.meshes);

    // Go find headlight position data inside dummies
    if(tag == NFS_3 || tag == NFS_4){
        for(auto &dummy : data.dummies){
            if(dummy.name.find("HFLO") != std::string::npos){
                leftHeadlight.cutOff = glm::cos(glm::radians(12.5f));
                leftHeadlight.position = leftHeadlight.initialPosition = dummy.position;
                leftHeadlight.colour = glm::vec3(1, 1, 1);
            }
            if(dummy.name.find("HFRE") != std::string::npos){
                rightHeadlight.cutOff = glm::cos(glm::radians(12.5f));
                rightHeadlight.position = rightHeadlight.initialPosition = dummy.position;
                rightHeadlight.colour = glm::vec3(1, 1, 1);
            }
            // TRLN, TRRN for tail lights
        }
    } else {
        leftHeadlight.cutOff = rightHeadlight.cutOff = glm::cos(glm::radians(12.5f));
        leftHeadlight.position =  rightHeadlight.position = carBodyModel.position;
        leftHeadlight.colour = rightHeadlight.colour = glm::vec3(1, 1, 1);
    }

    // Set car colour
    if(!data.colours.empty()){
        int randomColourIdx = (int) Utils::RandomFloat(0.f, (float) data.colours.size());
        colour = data.colours[randomColourIdx].colour;
    } else {
        colour = glm::vec3(1,1,1);
        //colour = glm::vec3(Utils::RandomFloat(0.f, 1.f), Utils::RandomFloat(0.f, 1.f), Utils::RandomFloat(0.f, 1.f));
    }

    glm::vec3 wheelDimensions = Utils::genDimensions(leftFrontWheelModel.m_vertices);
    wheelRadius = wheelDimensions.z;
    wheelWidth = wheelDimensions.x;

    // the chassis collision shape
    btCollisionShape* chassisShape = Utils::genCollisionBox(carBodyModel.m_vertices);
    m_collisionShapes.push_back(chassisShape);

    btCompoundShape* compound = new btCompoundShape();
    m_collisionShapes.push_back(compound);
    btTransform localTrans;
    localTrans.setIdentity();

    //Shift center of Mass (by 0 for now)
    if(tag == NFS_3 || tag == NFS_4){
        localTrans.setOrigin(btVector3(0.0,0.0,0));
    } else if (tag == NFS_3_PS1){
        localTrans.setOrigin(btVector3(0.0,0.1f,0));
    } else {
        localTrans.setOrigin(btVector3(0.0,0.05f,0));
    }

    compound->addChildShape(localTrans,chassisShape);

    float mass = 1750.0f;
    btVector3 localInertia(0,0,0);
    compound->calculateLocalInertia(mass,localInertia);

    // set initial location of vehicle in the world
    vehicleMotionState = new btDefaultMotionState(btTransform(btQuaternion(Utils::glmToBullet(carBodyModel.orientation)), btVector3(Utils::glmToBullet(carBodyModel.position))));
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
                leftFrontWheelModel = loaded_car_models[0];
                rightFrontWheelModel = loaded_car_models[0];
                leftRearWheelModel = loaded_car_models[0];
                rightRearWheelModel = loaded_car_models[0];
                loaded_car_models[1].enable();
                carBodyModel = loaded_car_models[1];
            }else if(loaded_car_models.size() < 20){
                loaded_car_models[2].enable();
                leftFrontWheelModel = loaded_car_models[2];
                rightFrontWheelModel = loaded_car_models[2];
                leftRearWheelModel = loaded_car_models[2];
                rightRearWheelModel = loaded_car_models[2];
                for(auto& car_model : loaded_car_models) {
                    if (car_model.m_name == "Medium Main Body Part") {
                        car_model.enable();
                        carBodyModel = car_model;
                    } else if (car_model.m_name.find("Medium") != std::string::npos && car_model.m_name.find("Wheel") == std::string::npos) {
                        if(car_model.m_name != loaded_car_models[2].m_name){
                            car_model.enable();
                            miscModels.emplace_back(car_model);
                        }
                    } else {
                        if(car_model.m_name != loaded_car_models[2].m_name){
                            miscModels.emplace_back(car_model);
                        }
                    }
                }
            } else {
                for(auto& car_model : loaded_car_models){
                    if(car_model.m_name == "High Main Body Part"){
                        car_model.enable();
                        carBodyModel = car_model;
                    } else if(car_model.m_name.find("High Front Left Wheel Part") != std::string::npos){
                        car_model.enable();
                        leftFrontWheelModel = car_model;
                    } else if(car_model.m_name.find("High Front Right Wheel Part") != std::string::npos){
                        car_model.enable();
                        rightFrontWheelModel = car_model;
                    } else if(car_model.m_name.find("High Rear Left Wheel Part") != std::string::npos){
                        car_model.enable();
                        leftRearWheelModel = car_model;
                    } else if(car_model.m_name.find("High Rear Right Wheel Part") != std::string::npos){
                        car_model.enable();
                        rightRearWheelModel = car_model;
                    } else if(car_model.m_name.find("High") != std::string::npos){ // Everything with "High" in the name is an extra body part, enable it
                        car_model.enable();
                        miscModels.emplace_back(car_model);
                    } else {
                        miscModels.emplace_back(car_model);
                    }
                }
            }
            break;
        case NFS_3:
            if(loaded_car_models.size() < 5) {
                carBodyModel = loaded_car_models[0];
                carBodyModel.enable();
                for(auto& car_model : loaded_car_models){
                    if(car_model.m_name.find("medium l front wheel") != std::string::npos){
                        car_model.enable();
                        leftFrontWheelModel = car_model;
                    } else if(car_model.m_name.find("medium r front wheel") != std::string::npos){
                        car_model.enable();
                        rightFrontWheelModel = car_model;
                    } else if(car_model.m_name.find("medium l rear wheel") != std::string::npos){
                        car_model.enable();
                        leftRearWheelModel = car_model;
                    } else if(car_model.m_name.find("medium r rear wheel") != std::string::npos){
                        car_model.enable();
                        rightRearWheelModel = car_model;
                    } else {
                        if(car_model.m_name != loaded_car_models[0].m_name){
                            miscModels.emplace_back(car_model);
                        }
                    }
                }
            } else {
                for(auto& car_model : loaded_car_models){
                    if(car_model.m_name == "high body"){
                        car_model.enable();
                        carBodyModel = car_model;
                    } else if(car_model.m_name.find("left front wheel") != std::string::npos){
                        car_model.enable();
                        leftFrontWheelModel = car_model;
                    } else if(car_model.m_name.find("right front wheel") != std::string::npos){
                        car_model.enable();
                        rightFrontWheelModel = car_model;
                    } else if(car_model.m_name.find("left rear wheel") != std::string::npos){
                        car_model.enable();
                        leftRearWheelModel = car_model;
                    } else if(car_model.m_name.find("right rear wheel") != std::string::npos){
                        car_model.enable();
                        rightRearWheelModel = car_model;
                    } else {
                        miscModels.emplace_back(car_model);
                    }
                }
            }
            break;
        case NFS_4:
            for(auto& car_model : loaded_car_models){
                if(car_model.m_name == ":HB"){
                    car_model.enable();
                    carBodyModel = car_model;
                } else if(car_model.m_name == ":HLRW"){
                    car_model.enable();
                    leftRearWheelModel = car_model;
                } else if(car_model.m_name == ":HLFW"){
                    car_model.enable();
                    leftFrontWheelModel = car_model;
                } else if(car_model.m_name == ":HRRW"){
                    car_model.enable();
                    rightRearWheelModel = car_model;
                } else if (car_model.m_name == ":HRFW"){
                    car_model.enable();
                    rightFrontWheelModel = car_model;
                } else if(car_model.m_name.find("O") != std::string::npos) {
                    car_model.enable();
                    miscModels.emplace_back(car_model);
                } else{
                    miscModels.emplace_back(car_model);
                }
            }
            break;
        case UNKNOWN:
            break;
        case NFS_5:
            break;
    }
}

void Car::setPosition(glm::vec3 position, glm::quat orientation){
    btTransform initialTransform;
    initialTransform.setOrigin(Utils::glmToBullet(position));
    initialTransform.setRotation(Utils::glmToBullet(orientation));
    m_carChassis->setWorldTransform(initialTransform);
    update();
}

Car::~Car() {
    leftFrontWheelModel.destroy();
    rightFrontWheelModel.destroy();
    leftRearWheelModel.destroy();
    rightRearWheelModel.destroy();
    carBodyModel.destroy();
    for (auto &car_model : miscModels) {
        car_model.destroy();
    }
    glDeleteTextures(1, &textureID);
}

void Car::update(){
    btTransform trans;
    vehicleMotionState->getWorldTransform(trans);
    carBodyModel.position = Utils::bulletToGlm(trans.getOrigin()) + (carBodyModel.initialPosition * glm::inverse(Utils::bulletToGlm(trans.getRotation())));
    carBodyModel.orientation = Utils::bulletToGlm(trans.getRotation());
    carBodyModel.update();

    // Might as well apply the body transform to the Miscellaneous models
    for(auto &misc_model : miscModels){
        misc_model.position = Utils::bulletToGlm(trans.getOrigin()) + (misc_model.initialPosition * glm::inverse(Utils::bulletToGlm(trans.getRotation())));
        misc_model.orientation = Utils::bulletToGlm(trans.getRotation());
        misc_model.update();
    }

    // Update headlight direction vectors to match car body
    leftHeadlight.direction = Utils::bulletToGlm(m_vehicle->getForwardVector());
    rightHeadlight.direction = Utils::bulletToGlm(m_vehicle->getForwardVector());
    leftHeadlight.position = Utils::bulletToGlm(trans.getOrigin()) + (leftHeadlight.initialPosition * glm::inverse(Utils::bulletToGlm(trans.getRotation())));
    rightHeadlight.position = Utils::bulletToGlm(trans.getOrigin()) + (rightHeadlight.initialPosition * glm::inverse(Utils::bulletToGlm(trans.getRotation())));

    // Lets go update wheel geometry positions based on physics feedback
    for (int i = 0; i <m_vehicle->getNumWheels(); i++) {
        m_vehicle->updateWheelTransform(i, true);trans = m_vehicle->getWheelInfo(i).m_worldTransform;
        switch(i){
            case 0:
                leftFrontWheelModel.position = Utils::bulletToGlm(trans.getOrigin());
                leftFrontWheelModel.orientation = Utils::bulletToGlm(trans.getRotation());
                leftFrontWheelModel.update();
                break;
            case 1:
                rightFrontWheelModel.position = Utils::bulletToGlm(trans.getOrigin());
                rightFrontWheelModel.orientation = Utils::bulletToGlm(trans.getRotation());
                rightFrontWheelModel.update();
                break;
            case 2:
                leftRearWheelModel.position = Utils::bulletToGlm(trans.getOrigin());
                leftRearWheelModel.orientation = Utils::bulletToGlm(trans.getRotation());
                leftRearWheelModel.update();
                break;
            case 3:
                rightRearWheelModel.position = Utils::bulletToGlm(trans.getOrigin());
                rightRearWheelModel.orientation = Utils::bulletToGlm(trans.getRotation());
                rightRearWheelModel.update();
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

void Car::update(btDynamicsWorld* dynamicsWorld) {
    // Update car
    update();
    // Update raycasts
    genRaycasts(dynamicsWorld);
}


void Car::genRaycasts(btDynamicsWorld* dynamicsWorld){
    btTransform trans;
    vehicleMotionState->getWorldTransform(trans);
    glm::vec3 carBodyPosition = Utils::bulletToGlm(trans.getOrigin());

    // Get base vectors
    glm::vec3 carUp = carBodyModel.ModelMatrix * glm::vec4(0,1,0,0);
    glm::vec3 carForward = Utils::bulletToGlm(m_vehicle->getForwardVector());

    btCollisionWorld::ClosestRayResultCallback *rayCallbacks[kNumRangefinders];
    glm::vec3 castVectors[kNumRangefinders];

    for(uint8_t rangeIdx = 0; rangeIdx < kNumRangefinders; ++rangeIdx){
        // Calculate base vector from -90 + (rangeIdx * kAngleBetweenRays) from car forward vector
        castVectors[rangeIdx] = carForward * glm::normalize(glm::quat(glm::vec3(0, glm::radians(-90.f + (rangeIdx * kAngleBetweenRays)), 0)));
        // Calculate where the ray will cast out to
        castPositions[rangeIdx] = carBodyPosition + (castVectors[rangeIdx] * kCastDistances[rangeIdx]);
        rayCallbacks[rangeIdx] = new btCollisionWorld::ClosestRayResultCallback(Utils::glmToBullet(carBodyPosition), Utils::glmToBullet(castPositions[rangeIdx]));
        // Don't Raycast against other opponents for now. Ghost through them. Only interested in VROAD edge.
        rayCallbacks[rangeIdx]->m_collisionFilterMask = COL_VROAD;
        // Perform the raycast
        dynamicsWorld->rayTest(Utils::glmToBullet(carBodyPosition), Utils::glmToBullet(castPositions[rangeIdx]), *rayCallbacks[rangeIdx]);
        // Check whether we hit anything
        if(rayCallbacks[rangeIdx]->hasHit()){
            rangefinders[rangeIdx] = glm::distance(carBodyPosition,  glm::vec3(rayCallbacks[rangeIdx]->m_hitPointWorld.getX(), rayCallbacks[rangeIdx]->m_hitPointWorld.getY(),rayCallbacks[rangeIdx]->m_hitPointWorld.getZ()));
        } else {
            rangefinders[rangeIdx] = kFarDistance;
        }
        delete rayCallbacks[rangeIdx];
    }
    upCastPosition = (carBodyPosition + (carUp * kCastDistance));
    downCastPosition = (carBodyPosition + (-carUp * kCastDistance));
    btCollisionWorld::ClosestRayResultCallback upRayCallback(Utils::glmToBullet(carBodyPosition), Utils::glmToBullet(upCastPosition));
    btCollisionWorld::ClosestRayResultCallback downRayCallback(Utils::glmToBullet(carBodyPosition), Utils::glmToBullet(downCastPosition));
    // Up raycast is used to check for flip over, and also whether inside VROAD
    upRayCallback.m_collisionFilterMask = downRayCallback.m_collisionFilterMask = COL_TRACK | COL_VROAD_CEIL;
    dynamicsWorld->rayTest(Utils::glmToBullet(carBodyPosition), Utils::glmToBullet(upCastPosition), upRayCallback);
    dynamicsWorld->rayTest(Utils::glmToBullet(carBodyPosition), Utils::glmToBullet(downCastPosition), downRayCallback);
    if(upRayCallback.hasHit()){
        upDistance = glm::distance(carBodyPosition,  glm::vec3(upRayCallback.m_hitPointWorld.getX(), upRayCallback.m_hitPointWorld.getY(),upRayCallback.m_hitPointWorld.getZ()));
    } else {
        upDistance = kFarDistance;
    }
    if(downRayCallback.hasHit()){
        downDistance = glm::distance(carBodyPosition,  glm::vec3(downRayCallback.m_hitPointWorld.getX(), downRayCallback.m_hitPointWorld.getY(),downRayCallback.m_hitPointWorld.getZ()));
    } else {
        downDistance = kFarDistance;
    }
}

void Car::applyAccelerationForce(bool accelerate, bool reverse)
{
    if (accelerate) {
        if(m_vehicle->getCurrentSpeedKmHour() < maxSpeed){
            gEngineForce = maxEngineForce;
            gBreakingForce = 0.f;
        } else {
            gEngineForce = 0.f;
        }
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

void Car::resetCar(glm::vec3 reset_position, glm::quat reset_orientation)
{
    setPosition(reset_position, reset_orientation);
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

    /*for (Model &mesh : carBodyModel) {
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

float Car::getRotY() {
    glm::quat orientation = carBodyModel.orientation;
    return glm::degrees(atan2(2*orientation.y*orientation.w - 2*orientation.x*orientation.z, 1 - 2*orientation.y*orientation.y - 2*orientation.z*orientation.z));
}

