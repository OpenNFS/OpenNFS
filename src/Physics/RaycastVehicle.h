#pragma once

#include <BulletDynamics/Vehicle/btRaycastVehicle.h>
#include <array>
#include <functional>

namespace OpenNFS {
    struct WheelContact {
        bool isInContact;
        btVector3 contactPointWS;
        btVector3 contactNormalWS;
        btScalar suspensionLength;
        btVector3 wheelDirectionWS;
        btVector3 wheelAxleWS;
        btRigidBody const *groundObject;
        int surfaceType; // NFS4 road surface ID

        // Derived values we compute
        btScalar suspensionCompression;
        btScalar suspensionForce;
    };

    // Callback to query surface type from your track collision
    using SurfaceQueryCallback = std::function<int(btVector3 const &point, btCollisionObject const *obj)>;

    class RaycastVehicle : public btRaycastVehicle {
      public:
        RaycastVehicle(btVehicleTuning const &tuning, btRigidBody *chassis, btVehicleRaycaster *raycaster,
                           SurfaceQueryCallback surfaceCallback = nullptr);

        // Override to capture raycast results and skip default force application
        void updateVehicle(btScalar step) override;

        // Perform raycasts without applying forces
        void updateWheelTransformsAndContacts();

        [[nodiscard]] WheelContact const &getWheelContact(int const wheelIndex) const {
            return m_wheelContacts[wheelIndex];
        }

      private:
        btVehicleRaycaster* m_vehicleRaycaster;
        std::array<WheelContact, 4> m_wheelContacts;
        SurfaceQueryCallback m_surfaceCallback;
        int m_indexForwardAxis{2};

        // Cache the raycast results before Bullet overwrites them
        void captureWheelContact(int wheelIndex);

        // Our own raycast that doesn't get cleared
        void performRaycast(int wheelIndex);

        // Query surface type from collision object
        int querySurfaceType(btVector3 const &point, btCollisionObject const *obj) const;
    };
} // namespace OpenNFS