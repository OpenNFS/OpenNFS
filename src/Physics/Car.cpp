#include "Car.h"

#include <utility>

#include "../Scene/Entity.h"
#include "CollisionMasks.h"

namespace OpenNFS {
    // Forward casts should extend further than L/R
    constexpr float kCastDistances[kNumRangefinders] = {1.f, 1.f, 1.f, 1.f, 1.f,  1.5f, 2.f, 3.f, 5.f, 5.f,
                                                        5.f, 3.f, 2.f, 2.f, 1.5f, 1.f,  1.f, 1.f, 1.f};

    Car::Car(LibOpenNFS::Car const &carData, GLuint const textureArrayID) : Car(carData) {
        renderInfo.textureArrayID = textureArrayID;
        renderInfo.isMultitexturedModel = true;
    }

    Car::Car(LibOpenNFS::Car carData) : assetData(std::move(carData)) {
        // Load in vehicle texture data to OpenGL
        if (!Config::get().vulkanRender) {
            this->_LoadTextures();
        }

        // Configure performance data
        this->_SetVehicleProperties();

        // Map mesh data to car data
        this->_SetModels(assetData.metadata.meshes);

        // Build the Bullet physics representation of the vehicle
        this->_GenPhysicsModel();
    }

    Car::~Car() {
        // And bullet collision shapes on heap
        m_collisionShapes.clear();
        // And the loaded GL textures
        if (renderInfo.isMultitexturedModel) {
            // TODO: Store number of textures so can pass correct parameter here
            glDeleteTextures(1, &renderInfo.textureArrayID);
        } else {
            glDeleteTextures(1, &renderInfo.textureID);
        }
    }

    void Car::Update(btDynamicsWorld *dynamicsWorld) {
        // Update car
        this->_UpdateMeshesToMatchPhysics();
        // Apply user input
        this->_ApplyInputs();
        // Update raycasts
        this->_GenRaycasts(dynamicsWorld);
    }

    void Car::ApplyAccelerationForce(bool const accelerate, bool const reverse) {
        if (accelerate) {
            if (m_vehicle->getCurrentSpeedKmHour() < vehicleProperties.maxSpeed) {
                vehicleState.gEngineForce = vehicleProperties.maxEngineForce;
                vehicleState.gBreakingForce = 0.f;
            } else {
                vehicleState.gEngineForce = 0.f;
            }
        } else if (reverse) {
            vehicleState.gEngineForce = -vehicleProperties.maxEngineForce;
            vehicleState.gBreakingForce = 0.f;
        } else {
            vehicleState.gEngineForce = 0.f;
        }
    }

    void Car::ApplySteeringRight(bool const apply) {
        vehicleState.steerRight = apply;
    }

    void Car::ApplySteeringLeft(bool const apply) {
        vehicleState.steerLeft = apply;
    }

    void Car::ApplyAbsoluteSteerAngle(float const targetAngle) {
        // Allow the update() method to directly utilise this targetAngle value
        vehicleProperties.absoluteSteer = true;
        // NN will always produce positive value, drop 0.5f from 0 -> 1 step output to allow -0.5 to 0.5
        float const finalSteering{targetAngle}; // - 0.5f;
        // Clamp value within steering extents
        vehicleState.gVehicleSteering =
            std::max(-vehicleProperties.steeringClamp, std::min(finalSteering, vehicleProperties.steeringClamp));
    }

    void Car::ApplyBrakingForce(bool const apply) {
        if (apply) {
            vehicleState.gBreakingForce = vehicleProperties.maxBreakingForce;
        } else {
            vehicleState.gBreakingForce = 0.f;
        }
    }

    void Car::SetPosition(glm::vec3 const position, glm::quat const orientation) {
        m_carChassis->clearForces();
        m_carChassis->setLinearVelocity(btVector3(0, 0, 0));
        m_carChassis->setAngularVelocity(btVector3(0, 0, 0));
        m_vehicle->resetSuspension();
        for (int wheelIdx = 0; wheelIdx < m_vehicle->getNumWheels(); ++wheelIdx) {
            // Synchronize the wheels with the (interpolated) chassis world transform
            m_vehicle->updateWheelTransform(wheelIdx, true);
        }

        // Create bullet transform of new positional and directional data
        btTransform const positionTransform{Utils::MakeTransform(position, orientation)};
        m_carChassis->setWorldTransform(positionTransform);

        // Update mesh positions to match new chassis transform
        this->_UpdateMeshesToMatchPhysics();
    }

    float Car::GetCarBodyOrientation() const {
        glm::quat const orientation{carBodyModel.orientation};
        return glm::degrees(atan2(2 * orientation.y * orientation.w - 2 * orientation.x * orientation.z,
                                  1 - 2 * orientation.y * orientation.y - 2 * orientation.z * orientation.z));
    }

    void Car::_UpdateMeshesToMatchPhysics() {
        btTransform trans;
        m_vehicleMotionState->getWorldTransform(trans);
        carBodyModel.position = Utils::bulletToGlm(trans.getOrigin()) +
                                (carBodyModel.initialPosition * glm::inverse(Utils::bulletToGlm(trans.getRotation())));
        carBodyModel.orientation = Utils::bulletToGlm(trans.getRotation());
        carBodyModel.UpdateMatrices();

        // Might as well apply the body transform to the Miscellaneous models
        for (auto &miscModel : miscModels) {
            miscModel.position = Utils::bulletToGlm(trans.getOrigin()) +
                                 (miscModel.initialPosition * glm::inverse(Utils::bulletToGlm(trans.getRotation())));
            miscModel.orientation = Utils::bulletToGlm(trans.getRotation());
            miscModel.UpdateMatrices();
        }

        // Update headlight direction vectors to match car body
        leftHeadlight.direction = Utils::bulletToGlm(m_vehicle->getForwardVector());
        rightHeadlight.direction = Utils::bulletToGlm(m_vehicle->getForwardVector());
        leftHeadlight.position =
            Utils::bulletToGlm(trans.getOrigin()) +
            (leftHeadlight.initialPosition * glm::inverse(Utils::bulletToGlm(trans.getRotation())));
        rightHeadlight.position =
            Utils::bulletToGlm(trans.getOrigin()) +
            (rightHeadlight.initialPosition * glm::inverse(Utils::bulletToGlm(trans.getRotation())));

        // Lets go update wheel geometry positions based on physics feedback
        for (int wheelIdx = 0; wheelIdx < m_vehicle->getNumWheels(); ++wheelIdx) {
            m_vehicle->updateWheelTransform(wheelIdx, true);
            trans = m_vehicle->getWheelInfo(wheelIdx).m_worldTransform;
            GLCarModel *wheelToUpdate{nullptr};
            switch (wheelIdx) {
            case FRONT_LEFT:
                wheelToUpdate = &leftFrontWheelModel;
                break;
            case FRONT_RIGHT:
                wheelToUpdate = &rightFrontWheelModel;
                break;
            case REAR_LEFT:
                wheelToUpdate = &leftRearWheelModel;
                break;
            case REAR_RIGHT:
                wheelToUpdate = &rightRearWheelModel;
                break;
            default:
                CHECK_F(false, "More than 4 wheels currently unsupported");
                break;
            }
            wheelToUpdate->position = Utils::bulletToGlm(trans.getOrigin());
            wheelToUpdate->orientation = Utils::bulletToGlm(trans.getRotation());
            wheelToUpdate->UpdateMatrices();
        }
    }

    void Car::_ApplyInputs() {
        if (!vehicleProperties.absoluteSteer) {
            // update front wheels steering value
            if (vehicleState.steerRight) {
                vehicleState.gVehicleSteering -= vehicleProperties.steeringIncrement;
                if (vehicleState.gVehicleSteering < -vehicleProperties.steeringClamp) {
                    vehicleState.gVehicleSteering = -vehicleProperties.steeringClamp;
                }
            } else if (vehicleState.steerLeft) {
                vehicleState.gVehicleSteering += vehicleProperties.steeringIncrement;
                if (vehicleState.gVehicleSteering > vehicleProperties.steeringClamp) {
                    vehicleState.gVehicleSteering = vehicleProperties.steeringClamp;
                }
            } else {
                if (vehicleState.gVehicleSteering > 0) {
                    vehicleState.gVehicleSteering -= vehicleProperties.steeringIncrement;
                } else if (vehicleState.gVehicleSteering < 0) {
                    vehicleState.gVehicleSteering += vehicleProperties.steeringIncrement;
                }
            }
        }

        // Set front wheels steering value
        m_vehicle->setSteeringValue(vehicleState.gVehicleSteering, FRONT_LEFT);
        m_vehicle->setSteeringValue(vehicleState.gVehicleSteering, FRONT_RIGHT);
        // Apply engine force to rear wheels (RWD only)
        m_vehicle->applyEngineForce(vehicleState.gEngineForce, REAR_LEFT);
        m_vehicle->setBrake(vehicleState.gBreakingForce, REAR_LEFT);
        m_vehicle->applyEngineForce(vehicleState.gEngineForce, REAR_RIGHT);
        m_vehicle->setBrake(vehicleState.gBreakingForce, REAR_RIGHT);
    }

    void Car::_LoadTextures() {
        std::stringstream carTexturePath;
        int width, height;
        carTexturePath << CAR_PATH << get_string(assetData.tag) << "/" << assetData.id;

        if (assetData.tag == NFSVersion::NFS_3 || assetData.tag == NFSVersion::NFS_4) {
            carTexturePath << "/car00.tga";
            renderInfo.textureID = ImageLoader::LoadImage(carTexturePath.str(), &width, &height, GL_CLAMP_TO_BORDER,
                                                          GL_LINEAR_MIPMAP_LINEAR);
        } else if (assetData.tag == NFSVersion::MCO) {
            std::stringstream car_alpha_texture_path;
            carTexturePath << "/Textures/0000.BMP";
            car_alpha_texture_path << CAR_PATH << get_string(assetData.tag) << "/" << assetData.id
                                   << "/Textures/0000-a.BMP";
            std::vector<uint8_t> imageData;
            if (ImageLoader::LoadBmpWithAlpha(carTexturePath.str().c_str(), car_alpha_texture_path.str().c_str(),
                                              imageData, &width, &height)) {
                glGenTextures(1, &renderInfo.textureID);
                glBindTexture(GL_TEXTURE_2D, renderInfo.textureID);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData.data());
                glGenerateMipmap(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, 0);
            }
        }
    }

    void Car::_GenPhysicsModel() {
        // Get the size of a wheel
        auto [wheelMinVertex, wheelMaxVertex] = Utils::GenDimensions(leftFrontWheelModel.m_vertices);
        auto wheelSize = glm::vec3((wheelMaxVertex.x - wheelMinVertex.x) / 2, (wheelMaxVertex.y - wheelMinVertex.y) / 2,
                                   (wheelMaxVertex.z - wheelMinVertex.z) / 2);
        vehicleProperties.wheelRadius = wheelSize.z;
        vehicleProperties.wheelWidth = wheelSize.x;

        // Generate the chassis collision mesh
        auto [bodyMinVertex, bodyMaxVertex] = Utils::GenDimensions(carBodyModel.m_vertices);
        // Drop size of car chassis vertically to avoid colliding with ground on suspension compression
        bodyMinVertex.y += 0.04f;
        btCollisionShape *chassisShape = new btBoxShape(Utils::glmToBullet((bodyMaxVertex - bodyMinVertex) / 2.f));
        m_collisionShapes.push_back(chassisShape);

        auto *compound = new btCompoundShape();
        btTransform localTrans;
        localTrans.setIdentity();

        // Shift center of Mass
        float centerOfMassShiftY;
        switch (assetData.tag) {
        case NFSVersion::NFS_3:
        case NFSVersion::NFS_4:
        case NFSVersion::MCO:
            centerOfMassShiftY = 0.f;
            break;
        case NFSVersion::NFS_3_PS1:
            centerOfMassShiftY = 0.1f;
            break;
        default:
            centerOfMassShiftY = 0.05f;
            break;
        }

        localTrans.setOrigin(btVector3(0.0, centerOfMassShiftY, 0));
        compound->addChildShape(localTrans, chassisShape);

        btVector3 localInertia(0, 0, 0);
        compound->calculateLocalInertia(vehicleProperties.mass, localInertia);
        m_collisionShapes.push_back(compound);

        // Set initial location of vehicle in the world
        m_vehicleMotionState = std::make_unique<btDefaultMotionState>(btTransform(
            btQuaternion(Utils::glmToBullet(carBodyModel.orientation)), Utils::glmToBullet(carBodyModel.position)));
        btRigidBody::btRigidBodyConstructionInfo cInfo(vehicleProperties.mass, m_vehicleMotionState.get(), compound,
                                                       localInertia);
        m_carChassis = std::make_unique<btRigidBody>(cInfo);

        // Abuse Entity system with a dummy entity that wraps the car pointer instead of a GL mesh
        // m_carChassis->setUserPointer(new Entity(-1, -1, tag, LibOpenNFS::EntityType::CAR, this, 0));
        m_carChassis->setDamping(0.2f, 0.2f);
        m_carChassis->setLinearVelocity(btVector3(0, 0, 0));
        m_carChassis->setAngularVelocity(btVector3(0, 0, 0));
        m_carChassis->setActivationState(DISABLE_DEACTIVATION);
    }

    void Car::_GenRaycasts(btDynamicsWorld const *dynamicsWorld) {
        btTransform trans;
        m_vehicleMotionState->getWorldTransform(trans);
        glm::vec3 const carBodyPosition{Utils::bulletToGlm(trans.getOrigin())};

        // Get base vectors
        glm::vec3 const carUp{carBodyModel.ModelMatrix * glm::vec4(0, 1, 0, 0)};
        glm::vec3 const carForward{Utils::bulletToGlm(m_vehicle->getForwardVector())};

        btCollisionWorld::ClosestRayResultCallback *rayCallbacks[kNumRangefinders];
        glm::vec3 castVectors[kNumRangefinders];

        for (uint8_t rangeIdx = 0; rangeIdx < kNumRangefinders; ++rangeIdx) {
            // Calculate base vector from -90 + (rangeIdx * kAngleBetweenRays) from car forward vector
            castVectors[rangeIdx] =
                carForward *
                glm::normalize(glm::quat(glm::vec3(0, glm::radians(-90.f + (rangeIdx * kAngleBetweenRays)), 0)));
            // Calculate where the ray will cast out to
            rangefinderInfo.castPositions[rangeIdx] =
                carBodyPosition + (castVectors[rangeIdx] * kCastDistances[rangeIdx]);
            rayCallbacks[rangeIdx] = new btCollisionWorld::ClosestRayResultCallback(
                Utils::glmToBullet(carBodyPosition), Utils::glmToBullet(rangefinderInfo.castPositions[rangeIdx]));
            // Don't Raycast against other opponents for now. Ghost through them. Only interested in VROAD edge.
            rayCallbacks[rangeIdx]->m_collisionFilterMask = CollisionMasks::COL_TRACK;
            // Perform the raycast
            dynamicsWorld->rayTest(Utils::glmToBullet(carBodyPosition),
                                   Utils::glmToBullet(rangefinderInfo.castPositions[rangeIdx]),
                                   *rayCallbacks[rangeIdx]);
            // Check whether we hit anything
            if (rayCallbacks[rangeIdx]->hasHit()) {
                rangefinderInfo.rangefinders[rangeIdx] =
                    glm::distance(carBodyPosition, Utils::bulletToGlm(rayCallbacks[rangeIdx]->m_hitPointWorld));
            } else {
                rangefinderInfo.rangefinders[rangeIdx] = kFarDistance;
            }
            delete rayCallbacks[rangeIdx];
        }
        rangefinderInfo.upCastPosition = (carBodyPosition + (carUp * kCastDistance));
        rangefinderInfo.downCastPosition = (carBodyPosition + (-carUp * kCastDistance));

        btCollisionWorld::ClosestRayResultCallback upRayCallback(Utils::glmToBullet(carBodyPosition),
                                                                 Utils::glmToBullet(rangefinderInfo.upCastPosition));
        btCollisionWorld::ClosestRayResultCallback downRayCallback(
            Utils::glmToBullet(carBodyPosition), Utils::glmToBullet(rangefinderInfo.downCastPosition));
        // Up raycast is used to check for flip over, and also whether inside VROAD
        upRayCallback.m_collisionFilterMask = downRayCallback.m_collisionFilterMask = COL_TRACK | COL_VROAD_CEIL;
        dynamicsWorld->rayTest(Utils::glmToBullet(carBodyPosition), Utils::glmToBullet(rangefinderInfo.upCastPosition),
                               upRayCallback);
        dynamicsWorld->rayTest(Utils::glmToBullet(carBodyPosition),
                               Utils::glmToBullet(rangefinderInfo.downCastPosition), downRayCallback);

        if (upRayCallback.hasHit()) {
            rangefinderInfo.upDistance =
                glm::distance(carBodyPosition, Utils::bulletToGlm(upRayCallback.m_hitPointWorld));
        } else {
            rangefinderInfo.upDistance = kFarDistance;
        }
        if (downRayCallback.hasHit()) {
            rangefinderInfo.downDistance =
                glm::distance(carBodyPosition, Utils::bulletToGlm(downRayCallback.m_hitPointWorld));
        } else {
            rangefinderInfo.downDistance = kFarDistance;
        }
    }

    // Take the list of Meshes returned by the car loader, and pull the High res wheels and body out for physics to
    // manipulate
    void Car::_SetModels(std::vector<LibOpenNFS::CarGeometry> &carGeometries) {
        switch (assetData.tag) {
        case NFSVersion::NFS_1:
            break;
        case NFSVersion::NFS_2_PS1:
        case NFSVersion::NFS_2_SE:
        case NFSVersion::NFS_2:
        case NFSVersion::NFS_3_PS1: {
            if (carGeometries.size() < 3) {
                GLCarModel wheelModel(carGeometries[0]);
                wheelModel.Enable();
                leftFrontWheelModel = wheelModel;
                rightFrontWheelModel = wheelModel;
                leftRearWheelModel = wheelModel;
                rightRearWheelModel = wheelModel;

                carBodyModel = GLCarModel(carGeometries[1]);
                carBodyModel.Enable();
            } else {
                for (auto &carGeometry : carGeometries) {
                    if (carGeometry.m_name == "High Main Body Part") {
                        carBodyModel = GLCarModel(carGeometry);
                        carBodyModel.Enable();
                    } else if (carGeometry.m_name.find("High Front Left Wheel Part") != std::string::npos) {
                        leftFrontWheelModel = GLCarModel(carGeometry);
                        leftFrontWheelModel.Enable();
                    } else if (carGeometry.m_name.find("High Front Right Wheel Part") != std::string::npos) {
                        rightFrontWheelModel = GLCarModel(carGeometry);
                        rightFrontWheelModel.Enable();
                    } else if (carGeometry.m_name.find("High Rear Left Wheel Part") != std::string::npos) {
                        leftRearWheelModel = GLCarModel(carGeometry);
                        leftRearWheelModel.Enable();
                    } else if (carGeometry.m_name.find("High Rear Right Wheel Part") != std::string::npos) {
                        rightRearWheelModel = GLCarModel(carGeometry);
                        rightRearWheelModel.Enable();
                    } else if (carGeometry.m_name.find("High") != std::string::npos) {
                        // Everything with "High" in the name is an extra body part, enable it
                        auto miscModel{GLCarModel(carGeometry)};
                        miscModel.Enable();
                        miscModels.push_back(miscModel);
                    } else {
                        miscModels.emplace_back(carGeometry);
                    }
                }
            }
        } break;
        case NFSVersion::NFS_3: {
            carBodyModel = GLCarModel(carGeometries[0]);
            carBodyModel.Enable();
            leftFrontWheelModel = GLCarModel(carGeometries[1]);
            leftFrontWheelModel.Enable();
            rightFrontWheelModel = GLCarModel(carGeometries[2]);
            rightFrontWheelModel.Enable();
            leftRearWheelModel = GLCarModel(carGeometries[3]);
            leftRearWheelModel.Enable();
            rightRearWheelModel = GLCarModel(carGeometries[4]);
            rightRearWheelModel.Enable();
            if (carGeometries.size() >= 5) {
                for (size_t partIdx = 5; partIdx < carGeometries.size(); ++partIdx) {
                    miscModels.emplace_back(carGeometries[partIdx]);
                }
            }
        } break;
        /*case NFSVersion::NFS_4:
            for (auto &carModel : carModels) {
                if (carModel.geometry->m_name == ":HB") {
                    carModel.enable();
                    carBodyModel = carModel;
                } else if (carModel.geometry->m_name == ":HLRW") {
                    carModel.enable();
                    leftRearWheelModel = carModel;
                } else if (carModel.geometry->m_name == ":HLFW") {
                    carModel.enable();
                    leftFrontWheelModel = carModel;
                } else if (carModel.geometry->m_name == ":HRRW") {
                    carModel.enable();
                    rightRearWheelModel = carModel;
                } else if (carModel.geometry->m_name == ":HRFW") {
                    carModel.enable();
                    rightFrontWheelModel = carModel;
                } else if (carModel.geometry->m_name.find('O') != std::string::npos) {
                    carModel.enable();
                    miscModels.emplace_back(carModel);
                } else {
                    miscModels.emplace_back(carModel);
                }
            }
            break;
        case NFSVersion::NFS_4_PS1:
            for (auto &carModel : carModels) {
                if (carModel.geometry->m_name.find("Right Body High") != std::string::npos) {
                    carModel.enable();
                    carBodyModel = carModel;
                } else if (carModel.geometry->m_name.find("Rear Left Wheel") != std::string::npos) {
                    carModel.enable();
                    leftRearWheelModel = carModel;
                } else if (carModel.geometry->m_name.find("Front Left Tire") != std::string::npos) {
                    carModel.enable();
                    leftFrontWheelModel = carModel;
                } else if (carModel.geometry->m_name.find("Rear Right Wheel") != std::string::npos) {
                    carModel.enable();
                    rightRearWheelModel = carModel;
                } else if (carModel.geometry->m_name.find("Front Right Tire") != std::string::npos) {
                    carModel.enable();
                    rightFrontWheelModel = carModel;
                } else if (carModel.geometry->m_name.find("Left Body High") != std::string::npos) {
                    carModel.enable();
                    miscModels.emplace_back(carModel);
                } else {
                    miscModels.emplace_back(carModel);
                }
            }
            break;
        case NFSVersion::MCO:
            for (auto &carModel : carModels) {
                if (carModel.geometry->m_name == ":Hbody") {
                    carModel.enable();
                    carBodyModel = carModel;
                } else if (carModel.geometry->m_name == ":PPLRwheel") {
                    carModel.enable();
                    leftRearWheelModel = carModel;
                } else if (carModel.geometry->m_name == ":PPLFwheel") {
                    carModel.enable();
                    leftFrontWheelModel = carModel;
                } else if (carModel.geometry->m_name == ":PPRRwheel") {
                    carModel.enable();
                    rightRearWheelModel = carModel;
                } else if (carModel.geometry->m_name == ":PPRFwheel") {
                    carModel.enable();
                    rightFrontWheelModel = carModel;
                } else if (carModel.geometry->m_name.find(":H") != std::string::npos) {
                    carModel.enable();
                    miscModels.emplace_back(carModel);
                } else {
                    miscModels.emplace_back(carModel);
                }
            }
            break;
        case NFSVersion::UNKNOWN:
            break;
        case NFSVersion::NFS_5: {
            for (auto &carModel : carModels) {
                if (carModel.geometry->m_name.find("Body_ig1") != std::string::npos) {
                    carModel.enable();
                    carBodyModel = carModel;
                } else if (carModel.geometry->m_name.find("WheelFront_fe1") != std::string::npos) {
                    carModel.enable();
                    leftFrontWheelModel = carModel;
                } else if (carModel.geometry->m_name.find("WheelFront_ig1") != std::string::npos) {
                    carModel.enable();
                    rightFrontWheelModel = carModel;
                } else if (carModel.geometry->m_name.find("WheelRear_fe1") != std::string::npos) {
                    carModel.enable();
                    leftRearWheelModel = carModel;
                } else if (carModel.geometry->m_name.find("WheelRear_ig1") != std::string::npos) {
                    carModel.enable();
                    rightRearWheelModel = carModel;
                } else {
                    // Enable all High LOD ig1 models
                    carModel.enabled = ((carModel.geometry->m_name.find("ig1") != std::string::npos) &&
        (carModel.geometry->m_name.find("Shadow") == std::string::npos)); miscModels.emplace_back(carModel);
                }
            }
        } break;*/
        default:
            CHECK_F(false, "Unsupported NFSVersion %s", get_string(assetData.tag).c_str());
        }

        // Go find headlight position data inside dummies
        if (assetData.tag == NFSVersion::NFS_3 || assetData.tag == NFSVersion::NFS_4) {
            for (auto &dummy : assetData.metadata.dummies) {
                if (dummy.name.find("HFLO") != std::string::npos) {
                    leftHeadlight.cutOff = glm::cos(glm::radians(12.5f));
                    leftHeadlight.position = leftHeadlight.initialPosition = dummy.position;
                    leftHeadlight.colour = glm::vec3(1, 1, 1);
                }
                if (dummy.name.find("HFRE") != std::string::npos) {
                    rightHeadlight.cutOff = glm::cos(glm::radians(12.5f));
                    rightHeadlight.position = rightHeadlight.initialPosition = dummy.position;
                    rightHeadlight.colour = glm::vec3(1, 1, 1);
                }
                // TRLN, TRRN for tail lights
            }
        } else {
            leftHeadlight.cutOff = rightHeadlight.cutOff = glm::cos(glm::radians(12.5f));
            leftHeadlight.position = rightHeadlight.position = carBodyModel.position;
            leftHeadlight.colour = rightHeadlight.colour = glm::vec3(1, 1, 1);
        }
    }

    void Car::_SetVehicleProperties() {
        // Load these from Carp.txt
        vehicleProperties.mass = 1750.f;
        vehicleProperties.maxSpeed = 100.f;
        vehicleProperties.maxEngineForce = 3000.f;
        vehicleProperties.maxBreakingForce = 1000.f;
        vehicleProperties.suspensionRestLength = 0.020;
        vehicleProperties.suspensionStiffness = 750.f;
        vehicleProperties.suspensionDamping = 200.f;
        vehicleProperties.suspensionCompression = 200.4f;
        vehicleProperties.wheelFriction = 0.45f;
        vehicleProperties.rollInfluence = 0.04f;
        vehicleProperties.steeringIncrement = 0.01f;
        vehicleProperties.steeringClamp = 0.15f;
        vehicleProperties.absoluteSteer = false;
        // Set car colour
        if (!assetData.metadata.colours.empty()) {
            uint32_t const randomColourIdx{(uint32_t)Utils::RandomFloat(0.f, (float)assetData.metadata.colours.size())};
            vehicleProperties.colour = assetData.metadata.colours[randomColourIdx].colour;
            vehicleProperties.colourSecondary = assetData.metadata.colours[randomColourIdx].colourSecondary;
        } else {
            vehicleProperties.colour =
                glm::vec4(Utils::RandomFloat(0.f, 1.f), Utils::RandomFloat(0.f, 1.f), Utils::RandomFloat(0.f, 1.f), 1.0f);
            vehicleProperties.colourSecondary =
                glm::vec4(Utils::RandomFloat(0.f, 1.f), Utils::RandomFloat(0.f, 1.f), Utils::RandomFloat(0.f, 1.f), 0.0f);
        }

        // State
        vehicleState.gEngineForce = 0.f;
        vehicleState.gBreakingForce = 100.f;
        vehicleState.gVehicleSteering = 0.f;
        vehicleState.steerRight = vehicleState.steerLeft = false;
    }
} // namespace OpenNFS
