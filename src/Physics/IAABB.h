#pragma once

#include <glm/vec3.hpp>

class AABB;

class IAABB {
public:
    virtual ~IAABB()             = default;
    [[nodiscard]] virtual AABB GetAABB() const = 0;
    [[nodiscard]] virtual glm::vec3 GetVelocity() const { return glm::vec3(0.0f); }
};
