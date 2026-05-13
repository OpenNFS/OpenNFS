#include "RaycastVehicle.h"

namespace OpenNFS {
    RaycastVehicle::RaycastVehicle(btVehicleTuning const &tuning, btRigidBody *chassis, btVehicleRaycaster *raycaster,
                                   SurfaceQueryCallback const &surfaceCallback)
        : btRaycastVehicle(tuning, chassis, raycaster), m_vehicleRaycaster(raycaster), m_surfaceCallback(surfaceCallback) {
        m_wheelContacts.fill({});
    }

    // btRaycastVehicle::updateVehicle() clone, with friction removed
    void RaycastVehicle::updateVehicle(btScalar step) {
        for (int i = 0; i < getNumWheels(); i++) {
            updateWheelTransform(i, false);
        }
        for (int i = 0; i < getNumWheels(); i++) {
            rayCast(m_wheelInfo[i]);
        }

        // Apply suspension force
        updateSuspension(step);

        for (int i = 0; i < getNumWheels(); i++) {
            btWheelInfo &wheel = m_wheelInfo[i];
            btScalar suspensionForce = wheel.m_wheelsSuspensionForce;
            if (suspensionForce > wheel.m_maxSuspensionForce) {
                suspensionForce = wheel.m_maxSuspensionForce;
            }
            btVector3 impulse = wheel.m_raycastInfo.m_contactNormalWS * suspensionForce * step;
            btVector3 relpos = wheel.m_raycastInfo.m_contactPointWS - getRigidBody()->getCenterOfMassPosition();
            getRigidBody()->applyImpulse(impulse, relpos);

            btVector3 relpos_hardpoint = wheel.m_raycastInfo.m_hardPointWS - getRigidBody()->getCenterOfMassPosition();
            btVector3 vel = getRigidBody()->getVelocityInLocalPoint(relpos_hardpoint);

            if (wheel.m_raycastInfo.m_isInContact) {
                btTransform const &chassisWorldTransform = getChassisWorldTransform();

                btVector3 fwd(chassisWorldTransform.getBasis()[0][m_indexForwardAxis],
                              chassisWorldTransform.getBasis()[1][m_indexForwardAxis],
                              chassisWorldTransform.getBasis()[2][m_indexForwardAxis]);

                btScalar proj = fwd.dot(wheel.m_raycastInfo.m_contactNormalWS);
                fwd -= wheel.m_raycastInfo.m_contactNormalWS * proj;

                btScalar const proj2 = fwd.dot(vel);

                wheel.m_deltaRotation = (proj2 * step) / (wheel.m_wheelsRadius);
                wheel.m_rotation += wheel.m_deltaRotation;
            } else {
                wheel.m_rotation += wheel.m_deltaRotation;
                wheel.m_deltaRotation *= btScalar(0.99); // damping of rotation when not in contact
            }
        }
    }

    void RaycastVehicle::performRaycast(int wheelIndex) {
        btWheelInfo &wheel = m_wheelInfo[wheelIndex];

        // Compute raycast start/end
        btTransform chassisTrans = getChassisWorldTransform();

        btVector3 const wheelDirectionWS = chassisTrans.getBasis() * wheel.m_wheelDirectionCS;
        btVector3 const wheelAxleWS = chassisTrans.getBasis() * wheel.m_wheelAxleCS;

        wheel.m_raycastInfo.m_wheelDirectionWS = wheelDirectionWS;
        wheel.m_raycastInfo.m_wheelAxleWS = wheelAxleWS;
        wheel.m_raycastInfo.m_isInContact = false;

        btVector3 const source = wheel.m_raycastInfo.m_hardPointWS;
        btVector3 const target = source + wheelDirectionWS * (wheel.getSuspensionRestLength() + wheel.m_wheelsRadius);

        // Use the raycaster
        btVehicleRaycaster::btVehicleRaycasterResult rayResult;
        void *object = m_vehicleRaycaster->castRay(source, target, rayResult);

        wheel.m_raycastInfo.m_groundObject = nullptr;

        if (object) {
            wheel.m_raycastInfo.m_isInContact = true;
            wheel.m_raycastInfo.m_contactPointWS = rayResult.m_hitPointInWorld;
            wheel.m_raycastInfo.m_contactNormalWS = rayResult.m_hitNormalInWorld;
            wheel.m_raycastInfo.m_groundObject = static_cast<btRigidBody *>(object);

            btScalar const hitDistance = (wheel.m_raycastInfo.m_hardPointWS - rayResult.m_hitPointInWorld).length();
            wheel.m_raycastInfo.m_suspensionLength = hitDistance - wheel.m_wheelsRadius;

            // Clamp suspension
            btScalar const minSusp = wheel.getSuspensionRestLength() - wheel.m_maxSuspensionTravelCm;
            btScalar const maxSusp = wheel.getSuspensionRestLength() + wheel.m_maxSuspensionTravelCm;
            wheel.m_raycastInfo.m_suspensionLength = btClamped(wheel.m_raycastInfo.m_suspensionLength, minSusp, maxSusp);
        } else {
            wheel.m_raycastInfo.m_suspensionLength = wheel.getSuspensionRestLength();
        }

        // Cache the raycast results before Bullet overwrites them
        WheelContact &contact = m_wheelContacts[wheelIndex];

        contact.isInContact = wheel.m_raycastInfo.m_isInContact;
        contact.contactPointWS = wheel.m_raycastInfo.m_contactPointWS;
        contact.contactNormalWS = wheel.m_raycastInfo.m_contactNormalWS;
        contact.suspensionLength = wheel.m_raycastInfo.m_suspensionLength;
        contact.wheelDirectionWS = wheel.m_raycastInfo.m_wheelDirectionWS;
        contact.wheelAxleWS = wheel.m_raycastInfo.m_wheelAxleWS;
        contact.groundObject = static_cast<btRigidBody const *>(wheel.m_raycastInfo.m_groundObject);

        // Query surface type if we have a callback and hit something
        if (contact.isInContact && contact.groundObject) {
            contact.surfaceType = querySurfaceType(contact.contactPointWS, contact.groundObject);
        } else {
            contact.surfaceType = 0; // Default road
        }
    }

    int RaycastVehicle::querySurfaceType(btVector3 const &point, btCollisionObject const *obj) const {
        if (m_surfaceCallback) {
            return m_surfaceCallback(point, obj);
        }

        // TODO: Set user pointer to contain built 2D map of surface data.
        //  We can then look it up by the worldspace x,y,z of each wheel!
        if (obj->getUserPointer()) {
            // return reinterpret_cast<TrackSurfaceData*>(obj->getUserPointer())->surfaceType;
        }

        return 0; // Default asphalt
    }

    void RaycastVehicle::updateWheelTransformsAndContacts() {
        for (int i = 0; i < getNumWheels(); i++) {
            performRaycast(i);
        }
    }
} // namespace OpenNFS
