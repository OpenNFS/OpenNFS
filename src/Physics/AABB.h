#pragma once

#include <algorithm>
#include <glm/glm.hpp>

class AABB
{
private:
    float _CalculateSurfaceArea() const
    {
        return 2.0f * (GetWidth() * GetHeight() + GetWidth() * GetDepth() + GetHeight() * GetDepth());
    }

public:
    glm::vec3 min;
    glm::vec3 max;
    float surfaceArea;
    glm::vec3 position;

    explicit AABB() = default;

    AABB(glm::vec3 minVertex, glm::vec3 maxVertex, glm::vec3 centerPosition) : min(minVertex), max(maxVertex), position(centerPosition)
    {
        surfaceArea = this->_CalculateSurfaceArea();
    }

    bool Overlaps(const AABB& other) const
    {
        return max.x > other.min.x && min.x < other.max.x &&
               max.y > other.min.y && min.y < other.max.y &&
               max.z > other.min.z && min.z < other.max.z;
    }

    bool Contains(const AABB& other) const
    {
        return other.min.x >= min.x && other.max.x <= max.x &&
               other.min.y >= min.y && other.max.y <= max.y &&
               other.min.z >= min.z && other.max.z <= max.z;
    }

    AABB Merge(const AABB& other) const
    {
        return AABB(
                glm::vec3(std::min(min.x, other.min.x), std::min(min.y, other.min.y), std::min(min.z, other.min.z)),
                glm::vec3(std::max(max.x, other.max.x), std::max(max.y, other.max.y), std::max(max.z, other.max.z)),
                (position + other.position)/2.f
        );
    }

    float GetWidth() const { return max.x - min.x; }
    float GetHeight() const { return max.y - min.y; }
    float GetDepth() const { return max.z - min.z; }
};
