#pragma once

#include <glm/glm.hpp>

class AABB {
public:
    explicit AABB() = default;
    AABB(glm::vec3 minVertex, glm::vec3 maxVertex, glm::vec3 centerPosition);

    bool Overlaps(const AABB& other) const;
    bool Contains(const AABB& other) const;
    AABB Merge(const AABB& other) const;
    float GetWidth() const;
    float GetHeight() const;
    float GetDepth() const;

    glm::vec3 min;
    glm::vec3 max;
    float surfaceArea;
    glm::vec3 position;

private:
    float _CalculateSurfaceArea() const;
};
