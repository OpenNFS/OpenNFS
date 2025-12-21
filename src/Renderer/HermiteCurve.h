#pragma once

#include <glm/vec3.hpp>
#include <iostream>
#include <vector>

class HermiteCurve {
  public:
    HermiteCurve(std::vector<glm::vec3> const &points, float tension, float bias);
    HermiteCurve() = default;
    [[nodiscard]] glm::vec3 GetPointAt(float t) const;
    [[nodiscard]] static glm::vec3 Interpolate(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, float t, float tension, float bias);
    [[nodiscard]] size_t GetLength() const {
        return m_points.size();
    }

  private:
    std::vector<glm::vec3> m_points;
    float m_tension = 0.f;
    float m_bias = 0.f;
};
