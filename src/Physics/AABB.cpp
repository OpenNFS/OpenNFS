#include "AABB.h"

#include <algorithm>

AABB::AABB(glm::vec3 minVertex, glm::vec3 maxVertex, glm::vec3 centerPosition) : min(minVertex), max(maxVertex), position(centerPosition) {
    surfaceArea = this->_CalculateSurfaceArea();
}

bool AABB::Overlaps(const AABB &other) const {
    return max.x > other.min.x && min.x < other.max.x && max.y > other.min.y && min.y < other.max.y && max.z > other.min.z && min.z < other.max.z;
}

bool AABB::Contains(const AABB &other) const {
    return other.min.x >= min.x && other.max.x <= max.x && other.min.y >= min.y && other.max.y <= max.y && other.min.z >= min.z && other.max.z <= max.z;
}

AABB AABB::Merge(const AABB &other) const {
    return {glm::vec3(std::min(min.x, other.min.x), std::min(min.y, other.min.y), std::min(min.z, other.min.z)),
            glm::vec3(std::max(max.x, other.max.x), std::max(max.y, other.max.y), std::max(max.z, other.max.z)), (position + other.position) / 2.f};
}

float AABB::GetWidth() const {
    return max.x - min.x;
}

float AABB::GetHeight() const {
    return max.y - min.y;
}

float AABB::GetDepth() const {
    return max.z - min.z;
}

float AABB::_CalculateSurfaceArea() const {
    return 2.0f * (GetWidth() * GetHeight() + GetWidth() * GetDepth() + GetHeight() * GetDepth());
}