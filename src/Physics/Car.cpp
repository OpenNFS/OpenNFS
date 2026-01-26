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
        this->_SetVehicleColour();

        // Map mesh data to car data
        this->_SetModels(assetData.metadata.meshes);

        // Build the Bullet physics representation of the vehicle
        this->_GenPhysicsShapes();
    }

    Car::~Car() {
        // Clean-up bullet collision shapes on heap
        for (size_t i = 0; i < m_collisionShapes.size(); i++) {
            delete m_collisionShapes[i];
        }
        m_collisionShapes.clear();
        // And the loaded GL textures
        if (renderInfo.isMultitexturedModel) {
            glDeleteTextures(1, &renderInfo.textureArrayID);
        } else {
            glDeleteTextures(1, &renderInfo.textureID);
        }
    }

    void Car::Update(btDynamicsWorld const *dynamicsWorld, float const dt) {
        // Update car
        btTransform trans;
        m_vehicleMotionState->getWorldTransform(trans);
        this->UpdateMeshesToTransform(trans);

        // Apply user input
        this->_ApplyInputs();

        // Update raycasts
        this->_GenRaycasts(dynamicsWorld);

        switch (physicsModel) {
        case PhysicsModel::BULLET:
            break;
        case PhysicsModel::NFS4_PC:
            m_nfs4VehiclePhysics->Update(dt);
            break;
        }
    }

    void Car::ApplyAccelerationForce(bool const accelerate, bool const reverse) {
        switch (physicsModel) {
        case PhysicsModel::BULLET:
            if (accelerate) {
                if (m_vehicle->getCurrentSpeedKmHour() < assetData.physicsData.maxSpeed) {
                    vehicleState.gEngineForce = assetData.physicsData.maxEngineForce;
                    vehicleState.gBrakingForce = 0.f;
                } else {
                    vehicleState.gEngineForce = 0.f;
                }
            } else if (reverse) {
                vehicleState.gEngineForce = -assetData.physicsData.maxEngineForce;
                vehicleState.gBrakingForce = 0.f;
            } else {
                vehicleState.gEngineForce = 0.f;
            }
            break;
        case PhysicsModel::NFS4_PC:
            vehicleState.throttlePedal = accelerate ? 1.0f : 0.f;
            // Reverse is handled with gears. Reuse reverse key for brakes.
            vehicleState.brakePedal = reverse ? 1.0f : 0.f;
            break;
        }
    }

    void Car::ApplySteeringRight(bool const apply) {
        vehicleState.steerRight = apply;
    }

    void Car::ApplySteeringLeft(bool const apply) {
        vehicleState.steerLeft = apply;
    }

    void Car::ApplyGearChange(bool const up, bool const down) {
        if (up && vehicleState.requestedGear < Gear::GEAR_6) {
            vehicleState.requestedGear = static_cast<Gear>(static_cast<int>(vehicleState.requestedGear) + 1);
        } else if (down && vehicleState.requestedGear > Gear::REVERSE) {
            vehicleState.requestedGear = static_cast<Gear>(static_cast<int>(vehicleState.requestedGear) - 1);
        }
    }

    void Car::ApplyHandbrakes(bool const apply) {
        vehicleState.handbrake = apply;
    }

    void Car::ToggleHeadlights() {
        vehicleState.headlightsActive = !vehicleState.headlightsActive;
    }

    void Car::ApplyAbsoluteSteerAngle(float const targetAngle) {
        switch (physicsModel) {
        case PhysicsModel::BULLET: {
            // Allow the update() method to directly utilise this targetAngle value
            assetData.physicsData.absoluteSteer = true;
            // NN will always produce positive value, drop 0.5f from 0 -> 1 step output to allow -0.5 to 0.5
            float const finalSteering{targetAngle}; // - 0.5f;
            // Clamp value within steering extents
            vehicleState.gVehicleSteering =
                std::max(-assetData.physicsData.steeringClamp, std::min(finalSteering, assetData.physicsData.steeringClamp));
        } break;
        case PhysicsModel::NFS4_PC:
            LOG(WARNING) << "Absolute steering mode unavailable with NFS4 PC Physics Model";
            break;
        }
    }

    void Car::ApplyBrakingForce(bool const apply) {
        switch (physicsModel) {
        case PhysicsModel::BULLET:
            if (apply) {
                vehicleState.gBrakingForce = assetData.physicsData.maxBreakingForce;
            } else {
                vehicleState.gBrakingForce = 0.f;
            }
            break;
        case PhysicsModel::NFS4_PC:
            vehicleState.brakePedal = apply ? 1.0f : 0;
            break;
        }
        vehicleState.taillightsActive = apply;
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
        this->UpdateMeshesToTransform(positionTransform);
    }

    float Car::GetCarBodyOrientation() const {
        glm::quat const orientation{carBodyModel.orientation};
        return glm::degrees(atan2(2 * orientation.y * orientation.w - 2 * orientation.x * orientation.z,
                                  1 - 2 * orientation.y * orientation.y - 2 * orientation.z * orientation.z));
    }

    void Car::UpdateMeshesToTransform(btTransform const &trans, bool const avoidPhysics) {
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

        if (avoidPhysics) {
            for (auto wheelIdx = 0; wheelIdx < 4; ++wheelIdx) {
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
                wheelToUpdate->position = wheelToUpdate->initialPosition;
                wheelToUpdate->UpdateMatrices();
            }
            return;
        }

        // Update headlight direction vectors to match car body
        glm::quat const carRotation{Utils::bulletToGlm(trans.getRotation())};
        leftHeadLight.direction = Utils::bulletToGlm(m_vehicle->getForwardVector());
        rightHeadLight.direction = Utils::bulletToGlm(m_vehicle->getForwardVector());
        leftHeadLight.position = Utils::bulletToGlm(trans.getOrigin()) + (carRotation * leftHeadLight.initialPosition);
        rightHeadLight.position = Utils::bulletToGlm(trans.getOrigin()) + (carRotation * rightHeadLight.initialPosition);
        leftTailLight.position = Utils::bulletToGlm(trans.getOrigin()) + (carRotation * leftTailLight.initialPosition);
        rightTailLight.position = Utils::bulletToGlm(trans.getOrigin()) + (carRotation * rightTailLight.initialPosition);

        // Lets go update wheel geometry positions based on physics feedback
        for (auto wheelIdx = 0; wheelIdx < m_vehicle->getNumWheels(); ++wheelIdx) {
            m_vehicle->updateWheelTransform(wheelIdx, true);
            btTransform wheelTrans = m_vehicle->getWheelInfo(wheelIdx).m_worldTransform;
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
            wheelToUpdate->position = Utils::bulletToGlm(wheelTrans.getOrigin());
            wheelToUpdate->orientation = Utils::bulletToGlm(wheelTrans.getRotation());
            wheelToUpdate->UpdateMatrices();
        }
    }

    void Car::_ApplyInputs() {

        switch (physicsModel) {
        case PhysicsModel::BULLET:
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
            m_vehicle->setBrake(vehicleState.gBrakingForce, REAR_LEFT);
            m_vehicle->applyEngineForce(vehicleState.gEngineForce, REAR_RIGHT);
            m_vehicle->setBrake(vehicleState.gBrakingForce, REAR_RIGHT);
            break;
        case PhysicsModel::NFS4_PC:
            m_nfs4VehiclePhysics->SetInput(vehicleState.throttlePedal, vehicleState.brakePedal, vehicleState.steerRight ? -1.0f : vehicleState.steerLeft ? 1.0f : 0,
                                           vehicleState.handbrake);
            // If the gear exceeds the supported max gear, drop it
            if (!m_nfs4VehiclePhysics->SetGear(vehicleState.requestedGear)) {
                vehicleState.requestedGear = static_cast<Gear>(static_cast<int>(vehicleState.requestedGear) - 1);
            }
            break;
        }

        leftTailLight.active = rightTailLight.active = vehicleState.taillightsActive;
    }

    void Car::_LoadTextures() {
        std::stringstream carTexturePath;
        int width, height;
        carTexturePath << CAR_PATH << magic_enum::enum_name(assetData.tag) << "/" << assetData.id;

        if (assetData.tag == NFSVersion::NFS_3 || assetData.tag == NFSVersion::NFS_4) {
            carTexturePath << "/car00.tga";
            renderInfo.textureID =
                ImageLoader::LoadImage(carTexturePath.str(), &width, &height, GL_CLAMP_TO_BORDER, GL_LINEAR_MIPMAP_LINEAR);
        } else if (assetData.tag == NFSVersion::MCO) {
            std::stringstream car_alpha_texture_path;
            carTexturePath << "/Textures/0000.BMP";
            car_alpha_texture_path << CAR_PATH << magic_enum::enum_name(assetData.tag) << "/" << assetData.id << "/Textures/0000-a.BMP";
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

    void Car::_GenPhysicsShapes() {
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
        float centerOfMassShiftY{};
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
        m_vehicleMotionState = std::make_unique<btDefaultMotionState>(
            btTransform(btQuaternion(Utils::glmToBullet(carBodyModel.orientation)), Utils::glmToBullet(carBodyModel.position)));
        btRigidBody::btRigidBodyConstructionInfo cInfo(assetData.physicsData.mass, m_vehicleMotionState.get(), compound, localInertia);
        m_carChassis = std::make_unique<btRigidBody>(cInfo);
        m_carChassis->setUserPointer(this);
        m_carChassis->setDamping(0.2f, 0.2f);
        m_carChassis->setLinearVelocity(btVector3(0, 0, 0));
        m_carChassis->setAngularVelocity(btVector3(0, 0, 0));
        m_carChassis->setActivationState(DISABLE_DEACTIVATION);
    }

    void Car::_GenRaycasts(btDynamicsWorld const *dynamicsWorld) {
        btTransform trans;
        m_vehicleMotionState->getWorldTransform(trans);
        glm::vec3 const carBodyPosition{Utils::bulletToGlm(trans.getOrigin())};
        btVector3 const btCarBodyPosition{Utils::glmToBullet(carBodyPosition)};

        // Get base vectors
        glm::vec3 const carUp{carBodyModel.ModelMatrix * glm::vec4(0, 1, 0, 0)};
        glm::vec3 const carForward{Utils::bulletToGlm(m_vehicle->getForwardVector())};

        for (uint8_t rangeIdx = 0; rangeIdx < kNumRangefinders; ++rangeIdx) {
            // Calculate base vector from -90 + (rangeIdx * kAngleBetweenRays) from car forward vector
            glm::vec3 const castVector =
                carForward * glm::normalize(glm::quat(glm::vec3(0, glm::radians(-90.f + (rangeIdx * kAngleBetweenRays)), 0)));
            // Calculate where the ray will cast out to
            rangefinderInfo.castPositions[rangeIdx] = carBodyPosition + (castVector * kCastDistances[rangeIdx]);
            btVector3 const btCastPosition{Utils::glmToBullet(rangefinderInfo.castPositions[rangeIdx])};

            btCollisionWorld::ClosestRayResultCallback rayCallback(btCarBodyPosition, btCastPosition);
            // Don't Raycast against other opponents for now. Ghost through them. Only interested in VROAD edge.
            rayCallback.m_collisionFilterMask = CollisionMasks::COL_TRACK;
            // Perform the raycast
            dynamicsWorld->rayTest(btCarBodyPosition, btCastPosition, rayCallback);
            // Check whether we hit anything
            if (rayCallback.hasHit()) {
                rangefinderInfo.rangefinders[rangeIdx] =
                    glm::distance(carBodyPosition, Utils::bulletToGlm(rayCallback.m_hitPointWorld));
            } else {
                rangefinderInfo.rangefinders[rangeIdx] = kFarDistance;
            }
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
        CHECK_F(!carGeometries.empty(), "No loaded vehicle geometry for ONFS to attach to a car");
        switch (assetData.tag) {
        case NFSVersion::NFS_1:
            break;
        case NFSVersion::NFS_2_PS1:
        case NFSVersion::NFS_2_SE:
        case NFSVersion::NFS_2:
        case NFSVersion::NFS_3_PS1: {
            if (carGeometries.size() < 3) {
                leftFrontWheelModel = GLCarModel(carGeometries[0]);
                leftFrontWheelModel.Enable();
                rightFrontWheelModel = GLCarModel(carGeometries[0]);
                rightFrontWheelModel.Enable();
                leftRearWheelModel = GLCarModel(carGeometries[0]);
                leftRearWheelModel.Enable();
                rightRearWheelModel = GLCarModel(carGeometries[0]);
                rightRearWheelModel.Enable();

                carBodyModel = GLCarModel(carGeometries[1]);
                carBodyModel.Enable();
            } else {
                for (auto &carGeometry : carGeometries) {
                    if (carGeometry.name == "High Main Body Part") {
                        carBodyModel = GLCarModel(carGeometry);
                        carBodyModel.Enable();
                    } else if (carGeometry.name.find("High Front Left Wheel Part") != std::string::npos) {
                        leftFrontWheelModel = GLCarModel(carGeometry);
                        leftFrontWheelModel.Enable();
                    } else if (carGeometry.name.find("High Front Right Wheel Part") != std::string::npos) {
                        rightFrontWheelModel = GLCarModel(carGeometry);
                        rightFrontWheelModel.Enable();
                    } else if (carGeometry.name.find("High Rear Left Wheel Part") != std::string::npos) {
                        leftRearWheelModel = GLCarModel(carGeometry);
                        leftRearWheelModel.Enable();
                    } else if (carGeometry.name.find("High Rear Right Wheel Part") != std::string::npos) {
                        rightRearWheelModel = GLCarModel(carGeometry);
                        rightRearWheelModel.Enable();
                    } else if (carGeometry.name.find("High") != std::string::npos) {
                        // Everything with "High" in the name is an extra body part, enable it
                        miscModels.emplace_back(carGeometry);
                        miscModels.back().Enable();
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
                if (carGeometry.name == ":HB") {
                    carBodyModel = GLCarModel(carGeometry);
                    carBodyModel.Enable();
                } else if (carGeometry.name == ":HLRW") {
                    leftRearWheelModel = GLCarModel(carGeometry);
                    leftRearWheelModel.Enable();
                } else if (carGeometry.name == ":HLFW") {
                    leftFrontWheelModel = GLCarModel(carGeometry);
                    leftFrontWheelModel.Enable();
                } else if (carGeometry.name == ":HRRW") {
                    rightRearWheelModel = GLCarModel(carGeometry);
                    rightRearWheelModel.Enable();
                } else if (carGeometry.name == ":HRFW") {
                    rightFrontWheelModel = GLCarModel(carGeometry);
                    rightFrontWheelModel.Enable();
                } else if (carGeometry.name.find('O') != std::string::npos) {
                    miscModels.emplace_back(carGeometry);
                } else {
                    miscModels.emplace_back(carGeometry);
                }
            }
            break;
        case NFSVersion::NFS_4_PS1:
            for (auto &carGeometry : carGeometries) {
                if (carGeometry.name.find("Right Body High") != std::string::npos) {
                    carBodyModel = GLCarModel(carGeometry);
                    carBodyModel.Enable();
                } else if (carGeometry.name.find("Rear Left Wheel") != std::string::npos) {
                    leftRearWheelModel = GLCarModel(carGeometry);
                    leftRearWheelModel.Enable();
                } else if (carGeometry.name.find("Front Left Tire") != std::string::npos) {
                    leftFrontWheelModel = GLCarModel(carGeometry);
                    leftFrontWheelModel.Enable();
                } else if (carGeometry.name.find("Rear Right Wheel") != std::string::npos) {
                    rightRearWheelModel = GLCarModel(carGeometry);
                    rightRearWheelModel.Enable();
                } else if (carGeometry.name.find("Front Right Tire") != std::string::npos) {
                    rightFrontWheelModel = GLCarModel(carGeometry);
                    rightFrontWheelModel.Enable();
                } else if (carGeometry.name.find("Left Body High") != std::string::npos) {
                    miscModels.emplace_back(carGeometry);
                    miscModels.back().Enable();
                } else {
                    miscModels.emplace_back(carGeometry);
                }
            }
            break;
        case NFSVersion::MCO:
            for (auto &carGeometry : carGeometries) {
                if (carGeometry.name == ":Hbody") {
                    carBodyModel = GLCarModel(carGeometry);
                    carBodyModel.Enable();
                } else if (carGeometry.name == ":PPLRwheel") {
                    leftRearWheelModel = GLCarModel(carGeometry);
                    leftRearWheelModel.Enable();
                } else if (carGeometry.name == ":PPLFwheel") {
                    leftFrontWheelModel = GLCarModel(carGeometry);
                    leftFrontWheelModel.Enable();
                } else if (carGeometry.name == ":PPRRwheel") {
                    rightRearWheelModel = GLCarModel(carGeometry);
                    rightRearWheelModel.Enable();
                } else if (carGeometry.name == ":PPRFwheel") {
                    rightFrontWheelModel = GLCarModel(carGeometry);
                    rightFrontWheelModel.Enable();
                } else if (carGeometry.name.find(":H") != std::string::npos) {
                    miscModels.emplace_back(carGeometry);
                } else {
                    miscModels.emplace_back(carGeometry);
                }
            }
            break;
        case NFSVersion::NFS_5: {
            for (auto &carGeometry : carGeometries) {
                if (carGeometry.name.find("Body_ig1") != std::string::npos) {
                    carBodyModel = GLCarModel(carGeometry);
                    carBodyModel.Enable();
                } else if (carGeometry.name.find("WheelFront_fe1") != std::string::npos) {
                    leftFrontWheelModel = GLCarModel(carGeometry);
                    leftFrontWheelModel.Enable();
                } else if (carGeometry.name.find("WheelFront_ig1") != std::string::npos) {
                    rightFrontWheelModel = GLCarModel(carGeometry);
                    rightFrontWheelModel.Enable();
                } else if (carGeometry.name.find("WheelRear_fe1") != std::string::npos) {
                    leftRearWheelModel = GLCarModel(carGeometry);
                    leftRearWheelModel.Enable();
                } else if (carGeometry.name.find("WheelRear_ig1") != std::string::npos) {
                    rightRearWheelModel = GLCarModel(carGeometry);
                    rightRearWheelModel.Enable();
                } else {
                    miscModels.emplace_back(carGeometry);
                    // Enable all High LOD ig1 models
                    if ((carGeometry.name.find("ig1") != std::string::npos) && (carGeometry.name.find("Shadow") == std::string::npos)) {
                        miscModels.back().Enable();
                    }
                }
            }
        } break;
        default:
            throw std::runtime_error(std::format("Unsupported NFSVersion {0}", magic_enum::enum_name(assetData.tag)));
        }

        // Go find headlight position data inside dummies
        if (assetData.tag == Utils::OneOf<NFSVersion::NFS_3, NFSVersion::NFS_4>()) {
            for (auto &dummy : assetData.metadata.dummies) {
                if (dummy.name.find("HFLO") != std::string::npos) {
                    leftHeadLight.innerCutOff = glm::cos(glm::radians(12.5f));
                    leftHeadLight.outerCutOff = glm::cos(glm::radians(17.5f));
                    leftHeadLight.position = leftHeadLight.initialPosition = dummy.position;
                    leftHeadLight.colour = glm::vec4(1, 1, 1, 0);
                }
                if (dummy.name.find("HFRE") != std::string::npos) {
                    rightHeadLight.innerCutOff = glm::cos(glm::radians(12.5f));
                    rightHeadLight.outerCutOff = glm::cos(glm::radians(17.5f));
                    rightHeadLight.position = rightHeadLight.initialPosition = dummy.position;
                    rightHeadLight.colour = glm::vec4(1, 1, 1, 0);
                }
                if (dummy.name.find("TRLN") != std::string::npos) {
                    leftTailLight.position = leftTailLight.initialPosition = dummy.position;
                    leftTailLight.colour = glm::vec4(1, 0, 0, 0);
                    rightTailLight.attenuation = glm::vec4(2, 2, 2, 0);
                }
                if (dummy.name.find("TRRN") != std::string::npos) {
                    rightTailLight.position = rightTailLight.initialPosition = dummy.position;
                    rightTailLight.colour = glm::vec4(1, 0, 0, 0);
                    rightTailLight.attenuation = glm::vec4(2, 2, 2, 0);
                }
            }
        } else {
            leftHeadLight.innerCutOff = rightHeadLight.innerCutOff = glm::cos(glm::radians(12.5f));
            leftHeadLight.outerCutOff = rightHeadLight.outerCutOff = glm::cos(glm::radians(17.5f));
            leftHeadLight.position = rightHeadLight.position = rightTailLight.position = leftTailLight.position = carBodyModel.position;
            leftHeadLight.colour = rightHeadLight.colour = rightTailLight.colour = leftTailLight.colour = glm::vec4(1, 0, 1, 0);
        }
    }

    void Car::_SetVehicleColour() {
        // Set car colour
        if (!assetData.metadata.colours.empty()) {
            uint32_t const randomColourIdx{(uint32_t)Utils::RandomFloat(0.f, (float)assetData.metadata.colours.size())};
            vehicleState.colour = assetData.metadata.colours[randomColourIdx].colour;
            vehicleState.colourSecondary = assetData.metadata.colours[randomColourIdx].colourSecondary;
        } else {
            vehicleState.colour = glm::vec4(Utils::RandomFloat(0.f, 1.f), Utils::RandomFloat(0.f, 1.f), Utils::RandomFloat(0.f, 1.f), 1.0f);
            vehicleState.colourSecondary =
                glm::vec4(Utils::RandomFloat(0.f, 1.f), Utils::RandomFloat(0.f, 1.f), Utils::RandomFloat(0.f, 1.f), 0.0f);
        }
    }

    void Car::SetPhysicsVehicle(std::unique_ptr<btRaycastVehicle> &&vehicle) {
        m_vehicle = std::move(vehicle);
        if (physicsModel == PhysicsModel::NFS4_PC) {
            auto raycastVehicle = dynamic_cast<RaycastVehicle *>(m_vehicle.get());
            if (!raycastVehicle) {
                throw std::runtime_error("Cannot cast vehicle pointer to custom Raycast vehicle");
            }
            m_nfs4VehiclePhysics = std::make_unique<NFS4VehiclePhysics>(raycastVehicle, m_carChassis.get(),
                                                                        NFS4PerformanceData::FromPhysicsData(assetData.physicsData));
        }
    }
} // namespace OpenNFS
