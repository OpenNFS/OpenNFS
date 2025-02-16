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
        this->_SetVehicleState();

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
            if (m_vehicle->getCurrentSpeedKmHour() < assetData.physicsData.maxSpeed) {
                vehicleState.gEngineForce = assetData.physicsData.maxEngineForce;
                vehicleState.gBreakingForce = 0.f;
            } else {
                vehicleState.gEngineForce = 0.f;
            }
        } else if (reverse) {
            vehicleState.gEngineForce = -assetData.physicsData.maxEngineForce;
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
        assetData.physicsData.absoluteSteer = true;
        // NN will always produce positive value, drop 0.5f from 0 -> 1 step output to allow -0.5 to 0.5
        float const finalSteering{targetAngle}; // - 0.5f;
        // Clamp value within steering extents
        vehicleState.gVehicleSteering =
            std::max(-assetData.physicsData.steeringClamp, std::min(finalSteering, assetData.physicsData.steeringClamp));
    }

    void Car::ApplyBrakingForce(bool const apply) {
        if (apply) {
            vehicleState.gBreakingForce = assetData.physicsData.maxBreakingForce;
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
        carBodyModel.position =
            Utils::bulletToGlm(trans.getOrigin()) + (carBodyModel.initialPosition * glm::inverse(Utils::bulletToGlm(trans.getRotation())));
        carBodyModel.orientation = Utils::bulletToGlm(trans.getRotation());
        carBodyModel.UpdateMatrices();

        // Might as well apply the body transform to the Miscellaneous models
        for (auto &miscModel : miscModels) {
            miscModel.position =
                Utils::bulletToGlm(trans.getOrigin()) + (miscModel.initialPosition * glm::inverse(Utils::bulletToGlm(trans.getRotation())));
            miscModel.orientation = Utils::bulletToGlm(trans.getRotation());
            miscModel.UpdateMatrices();
        }

        // Update headlight direction vectors to match car body
        leftHeadLight.direction = Utils::bulletToGlm(m_vehicle->getForwardVector());
        rightHeadLight.direction = Utils::bulletToGlm(m_vehicle->getForwardVector());
        leftHeadLight.position =
            Utils::bulletToGlm(trans.getOrigin()) + (leftHeadLight.initialPosition * glm::inverse(Utils::bulletToGlm(trans.getRotation())));
        rightHeadLight.position = Utils::bulletToGlm(trans.getOrigin()) +
                                  (rightHeadLight.initialPosition * glm::inverse(Utils::bulletToGlm(trans.getRotation())));
        leftTailLight.direction = Utils::bulletToGlm(-m_vehicle->getForwardVector());
        rightTailLight.direction = Utils::bulletToGlm(-m_vehicle->getForwardVector());
        leftTailLight.position =
            Utils::bulletToGlm(trans.getOrigin()) + (leftTailLight.initialPosition * glm::inverse(Utils::bulletToGlm(trans.getRotation())));
        rightTailLight.position = Utils::bulletToGlm(trans.getOrigin()) +
                                  (rightTailLight.initialPosition * glm::inverse(Utils::bulletToGlm(trans.getRotation())));

        // Lets go update wheel geometry positions based on physics feedback
        for (auto wheelIdx = 0; wheelIdx < m_vehicle->getNumWheels(); ++wheelIdx) {
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
        if (!assetData.physicsData.absoluteSteer) {
            // update front wheels steering value
            if (vehicleState.steerRight) {
                vehicleState.gVehicleSteering -= assetData.physicsData.steeringIncrement;
                if (vehicleState.gVehicleSteering < -assetData.physicsData.steeringClamp) {
                    vehicleState.gVehicleSteering = -assetData.physicsData.steeringClamp;
                }
            } else if (vehicleState.steerLeft) {
                vehicleState.gVehicleSteering += assetData.physicsData.steeringIncrement;
                if (vehicleState.gVehicleSteering > assetData.physicsData.steeringClamp) {
                    vehicleState.gVehicleSteering = assetData.physicsData.steeringClamp;
                }
            } else {
                if (vehicleState.gVehicleSteering > 0) {
                    vehicleState.gVehicleSteering -= assetData.physicsData.steeringIncrement;
                } else if (vehicleState.gVehicleSteering < 0) {
                    vehicleState.gVehicleSteering += assetData.physicsData.steeringIncrement;
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
            renderInfo.textureID =
                ImageLoader::LoadImage(carTexturePath.str(), &width, &height, GL_CLAMP_TO_BORDER, GL_LINEAR_MIPMAP_LINEAR);
        } else if (assetData.tag == NFSVersion::MCO) {
            std::stringstream car_alpha_texture_path;
            carTexturePath << "/Textures/0000.BMP";
            car_alpha_texture_path << CAR_PATH << get_string(assetData.tag) << "/" << assetData.id << "/Textures/0000-a.BMP";
            std::vector<uint8_t> imageData;
            if (ImageLoader::LoadBmpWithAlpha(carTexturePath.str().c_str(), car_alpha_texture_path.str().c_str(), imageData, &width,
                                              &height)) {
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
        assetData.physicsData.wheelRadius = wheelSize.z;
        assetData.physicsData.wheelWidth = wheelSize.x;

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
        compound->calculateLocalInertia(assetData.physicsData.mass, localInertia);
        m_collisionShapes.push_back(compound);

        // Set initial location of vehicle in the world
        m_vehicleMotionState = std::make_unique<btDefaultMotionState>(btTransform(
            btQuaternion(Utils::glmToBullet(carBodyModel.orientation)), Utils::glmToBullet(carBodyModel.position)));
        btRigidBody::btRigidBodyConstructionInfo cInfo(assetData.physicsData.mass, m_vehicleMotionState.get(), compound,
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
                carForward * glm::normalize(glm::quat(glm::vec3(0, glm::radians(-90.f + (rangeIdx * kAngleBetweenRays)), 0)));
            // Calculate where the ray will cast out to
            rangefinderInfo.castPositions[rangeIdx] = carBodyPosition + (castVectors[rangeIdx] * kCastDistances[rangeIdx]);
            rayCallbacks[rangeIdx] = new btCollisionWorld::ClosestRayResultCallback(
                Utils::glmToBullet(carBodyPosition), Utils::glmToBullet(rangefinderInfo.castPositions[rangeIdx]));
            // Don't Raycast against other opponents for now. Ghost through them. Only interested in VROAD edge.
            rayCallbacks[rangeIdx]->m_collisionFilterMask = CollisionMasks::COL_TRACK;
            // Perform the raycast
            dynamicsWorld->rayTest(Utils::glmToBullet(carBodyPosition), Utils::glmToBullet(rangefinderInfo.castPositions[rangeIdx]),
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
        btCollisionWorld::ClosestRayResultCallback downRayCallback(Utils::glmToBullet(carBodyPosition),
                                                                   Utils::glmToBullet(rangefinderInfo.downCastPosition));
        // Up raycast is used to check for flip over, and also whether inside VROAD
        upRayCallback.m_collisionFilterMask = downRayCallback.m_collisionFilterMask = COL_TRACK;
        dynamicsWorld->rayTest(Utils::glmToBullet(carBodyPosition), Utils::glmToBullet(rangefinderInfo.upCastPosition), upRayCallback);
        dynamicsWorld->rayTest(Utils::glmToBullet(carBodyPosition), Utils::glmToBullet(rangefinderInfo.downCastPosition), downRayCallback);

        if (upRayCallback.hasHit()) {
            rangefinderInfo.upDistance = glm::distance(carBodyPosition, Utils::bulletToGlm(upRayCallback.m_hitPointWorld));
        } else {
            rangefinderInfo.upDistance = kFarDistance;
        }
        if (downRayCallback.hasHit()) {
            rangefinderInfo.downDistance = glm::distance(carBodyPosition, Utils::bulletToGlm(downRayCallback.m_hitPointWorld));
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
        case NFSVersion::NFS_4:
            for (auto &carGeometry : carGeometries) {
                if (carGeometry.m_name == ":HB") {
                    carBodyModel = GLCarModel(carGeometry);
                    carBodyModel.Enable();
                } else if (carGeometry.m_name == ":HLRW") {
                    leftRearWheelModel = GLCarModel(carGeometry);
                    leftRearWheelModel.Enable();
                } else if (carGeometry.m_name == ":HLFW") {
                    leftFrontWheelModel = GLCarModel(carGeometry);
                    leftFrontWheelModel.Enable();
                } else if (carGeometry.m_name == ":HRRW") {
                    rightRearWheelModel = GLCarModel(carGeometry);
                    rightRearWheelModel.Enable();
                } else if (carGeometry.m_name == ":HRFW") {
                    rightFrontWheelModel = GLCarModel(carGeometry);;
                    rightFrontWheelModel.Enable();
                } else if (carGeometry.m_name.find('O') != std::string::npos) {
                    miscModels.emplace_back(carGeometry);
                } else {
                    miscModels.emplace_back(carGeometry);
                }
            }
            break;
        /*case NFSVersion::NFS_4_PS1:
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
                    leftHeadLight.cutOff = glm::cos(glm::radians(12.5f));
                    leftHeadLight.position = leftHeadLight.initialPosition = dummy.position;
                    leftHeadLight.colour = glm::vec4(1, 1, 1, 0);
                }
                if (dummy.name.find("HFRE") != std::string::npos) {
                    rightHeadLight.cutOff = glm::cos(glm::radians(12.5f));
                    rightHeadLight.position = rightHeadLight.initialPosition = dummy.position;
                    rightHeadLight.colour = glm::vec4(1, 1, 1, 0);
                }
                if (dummy.name.find("TRLN") != std::string::npos) {
                    leftTailLight.cutOff = glm::cos(glm::radians(12.5f));
                    leftTailLight.position = leftTailLight.initialPosition = dummy.position;
                    leftTailLight.colour = glm::vec4(1, 0, 0, 0);
                }
                if (dummy.name.find("TRRN") != std::string::npos) {
                    rightTailLight.cutOff = glm::cos(glm::radians(12.5f));
                    rightTailLight.position = rightTailLight.initialPosition = dummy.position;
                    rightTailLight.colour = glm::vec4(1, 0, 0, 0);
                }
            }

            lights.push_back(&leftTailLight);
            lights.push_back(&rightTailLight);
            lights.push_back(&leftHeadLight);
            lights.push_back(&rightHeadLight);
        } else {
            leftHeadLight.cutOff = rightHeadLight.cutOff = rightTailLight.cutOff = leftTailLight.cutOff = glm::cos(glm::radians(12.5f));
            leftHeadLight.position = rightHeadLight.position = rightTailLight.position = leftTailLight.position = carBodyModel.position;
            leftHeadLight.colour = rightHeadLight.colour = rightTailLight.colour = leftTailLight.colour = glm::vec4(1, 1, 1, 0);
        }
    }

    void Car::_SetVehicleState() {
        // Set car colour
        if (!assetData.metadata.colours.empty()) {
            uint32_t const randomColourIdx{(uint32_t)Utils::RandomFloat(0.f, (float)assetData.metadata.colours.size())};
            vehicleState.colour = assetData.metadata.colours[randomColourIdx].colour;
            vehicleState.colourSecondary = assetData.metadata.colours[randomColourIdx].colourSecondary;
        } else {
            vehicleState.colour =
                glm::vec4(Utils::RandomFloat(0.f, 1.f), Utils::RandomFloat(0.f, 1.f), Utils::RandomFloat(0.f, 1.f), 1.0f);
                vehicleState.colourSecondary =
                glm::vec4(Utils::RandomFloat(0.f, 1.f), Utils::RandomFloat(0.f, 1.f), Utils::RandomFloat(0.f, 1.f), 0.0f);

        }

        // State
        vehicleState.gEngineForce = 0.f;
        vehicleState.gBreakingForce = 100.f;
        vehicleState.gVehicleSteering = 0.f;
        vehicleState.steerRight = vehicleState.steerLeft = false;
    }
} // namespace OpenNFS
