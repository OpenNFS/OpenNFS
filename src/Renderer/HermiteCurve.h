#pragma once

#include <vector>
#include <cmath>
#include <iostream>
#include <glm/vec3.hpp>

class HermiteCurve {
public:
    HermiteCurve(const std::vector<glm::vec3> &points, float tension, float bias);
    HermiteCurve(){};
    glm::vec3 GetPointAt(float t);
    glm::vec3 Interpolate(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, float t, float tension, float bias);
    size_t GetLength() {
        return m_points.size();
    }

private:
    std::vector<glm::vec3> m_points;
    float m_tension = 0.f;
    float m_bias    = 0.f;
};
