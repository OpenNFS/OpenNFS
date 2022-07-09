#include "HermiteCurve.h"

// Phoboslab JS Impl: https://github.com/phoboslab/wipeout/blob/master/HermiteCurve3.js
// Based on http://paulbourke.net/miscellaneous/interpolation/
// tension: 1 high, 0 normal, -1 low
// bias: 0 is even, positive is towards first segment, negative towards the other
HermiteCurve::HermiteCurve(const std::vector<glm::vec3> &curvePoints, float curveTension, float curveBias) {
    m_points  = curvePoints;
    m_tension = curveTension;
    m_bias    = curveBias;
}

glm::vec3 HermiteCurve::GetPointAt(float t) {
    float point  = (m_points.size() - 1) * t;
    int intPoint = (int) point;

    float weight = point - intPoint;

    glm::vec3 point0 = m_points[intPoint == 0 ? intPoint : intPoint - 1];
    glm::vec3 point1 = m_points[intPoint];
    glm::vec3 point2 = m_points[intPoint > m_points.size() - 2 ? m_points.size() - 1 : intPoint + 1];
    glm::vec3 point3 = m_points[intPoint > m_points.size() - 3 ? m_points.size() - 1 : intPoint + 2];

    return Interpolate(point0, point1, point2, point3, weight, m_tension, m_bias);
}

glm::vec3 HermiteCurve::Interpolate(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, float t, float tension, float bias) {
    glm::vec3 m0 = (p1 - p0) * ((1 + bias) * (1 - tension) / 2) + (p2 - p1) * ((1 - bias) * (1 - tension) / 2);

    glm::vec3 m1 = (p2 - p1) * ((1 + bias) * (1 - tension) / 2) + (p3 - p2) * ((1 - bias) * (1 - tension) / 2);

    float t2 = t * t;
    float t3 = t2 * t;

    float h0 = 2 * t3 - 3 * t2 + 1;
    float h1 = t3 - 2 * t2 + t;
    float h2 = t3 - t2;
    float h3 = -2 * t3 + 3 * t2;

    return h0 * p1 + h1 * m0 + h2 * m1 + h3 * p2;
}
