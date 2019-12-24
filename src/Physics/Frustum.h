#pragma once

#include <glm/glm.hpp>
#include <array>

#include "AABB.h"

enum FrustumPlane : uint8_t
{
    LEFT,
    RIGHT,
    TOP,
    BOTTOM,
    NEAR,
    FAR,
    Length
};

class Frustum
{
public:
    Frustum() = default;
    explicit Frustum(const glm::mat4 &viewProjectionMatrix);
    void Update(const glm::mat4 &viewProjectionMatrix);
    bool CheckIntersection(const AABB &other) const;

private:
    std::array<glm::vec4, FrustumPlane::Length> frustumPlanes;
};