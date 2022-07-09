#pragma once

#include <glm/glm.hpp>
#include <array>

#include "AABB.h"

enum FrustumPlanes : uint8_t { LEFT = 0, RIGHT, BOTTOM, TOP, NEAR_P, FAR_P, Length, Combinations = Length * (Length - 1) / 2 };

class Frustum {
public:
    Frustum() = default;
    void Update(const glm::mat4 &projectionViewMatrix);
    bool CheckIntersection(const AABB &other) const;
    std::array<glm::vec3, 8> points;

private:
    template <FrustumPlanes i, FrustumPlanes j>
    struct ij2k {
        enum { k = i * (9 - i) / 2 + j - 1 };
    };

    template <FrustumPlanes a, FrustumPlanes b, FrustumPlanes c>
    glm::vec3 GetPlaneIntersection(const glm::vec3 *crosses) const;

    void _ExtractPlanes(const glm::mat4 &projectionViewMatrix);
    void _CalculatePlaneIntersections();

    std::array<glm::vec4, FrustumPlanes::Length> m_planes;
};