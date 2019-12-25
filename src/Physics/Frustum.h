#pragma once

#include <glm/glm.hpp>
#include <array>

#include "AABB.h"

enum FrustumPlane : uint8_t { LEFT, RIGHT, TOP, BOTTOM, NEAR_P, FAR_P, Length };

class Frustum
{
public:
    Frustum() = default;
    explicit Frustum(const glm::mat4 &viewProjectionMatrix);
    void Update(const glm::mat4 &viewProjectionMatrix);
    bool CheckIntersection(const AABB &other) const;

private:
    std::array<glm::vec4, static_cast<uint8_t>(FrustumPlane::Length)> frustumPlanes;
};