#pragma once

#define BIT(x) (1 << (x))

// Collision masks
enum CollisionMasks {
    COL_NOTHING       = 0,      // Collide with nothing
    COL_RAY           = BIT(0), // Collide with rays
    COL_CAR           = BIT(1), // Collide with cars
    COL_TRACK         = BIT(2), // Collide with track
    COL_VROAD         = BIT(3), // Collide with VROAD edge
    COL_VROAD_CEIL    = BIT(4), // Collide with VROAD ceiling
    COL_DYNAMIC_TRACK = BIT(5)  // Collide with roadsigns
};
