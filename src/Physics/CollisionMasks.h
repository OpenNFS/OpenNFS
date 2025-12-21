#pragma once

#define BIT(x) (1 << (x))

enum CollisionMasks {
    COL_NOTHING = 0,           // Collide with nothing
    COL_RAY = BIT(0),          // Collide with rays
    COL_CAR = BIT(1),          // Collide with cars
    COL_TRACK = BIT(2),        // Collide with track
    COL_DYNAMIC_TRACK = BIT(5) // Collide with roadsigns
};
