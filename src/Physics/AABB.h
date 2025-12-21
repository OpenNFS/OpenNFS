#pragma once

#include <glm/glm.hpp>

class AABB {
public:
    explicit AABB() = default;
    AABB(glm::vec3 minVertex, glm::vec3 maxVertex, glm::vec3 centerPosition);

    [[nodiscard]] bool Overlaps(const AABB& other) const;
    [[nodiscard]] bool Contains(const AABB& other) const;
    [[nodiscard]] AABB Merge(const AABB& other) const;
    [[nodiscard]] float GetWidth() const;
    [[nodiscard]] float GetHeight() const;
    [[nodiscard]] float GetDepth() const;

    glm::vec3 min;
    glm::vec3 max;
    float surfaceArea;

private:
    [[nodiscard]] float _CalculateSurfaceArea() const;
};
