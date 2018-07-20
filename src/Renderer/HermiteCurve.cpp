//
// Created by SYSTEM on 20/07/2018.
//


#include "HermiteCurve.h"

// Phoboslab JS Impl: https://github.com/phoboslab/wipeout/blob/master/HermiteCurve3.js
// Based on http://paulbourke.net/miscellaneous/interpolation/
// tension: 1 high, 0 normal, -1 low
// bias: 0 is even, positive is towards first segment, negative towards the other
HermiteCurve::HermiteCurve(std::vector<glm::vec3> curvePoints, float curveTension, float curveBias) {
    points = curvePoints;
    tension = curveTension;
    bias = curveBias;
}

glm::vec3 HermiteCurve::getPointAt(float t) {
    float point = (points.size() - 1) * t;
    int intPoint = (int) point;

    float weight = point - intPoint;

    glm::vec3 point0 = points[intPoint == 0 ? intPoint : intPoint - 1];
    glm::vec3 point1 = points[intPoint];
    glm::vec3 point2 = points[intPoint > points.size() - 2 ? points.size() - 1 : intPoint + 1];
    glm::vec3 point3 = points[intPoint > points.size() - 3 ? points.size() - 1 : intPoint + 2];

    glm::vec3 vector(0, 0, 0);
    // TODO: Use a vector based interpolation instead of this
    vector.x = Interpolate(point0.x, point1.x, point2.x, point3.x, weight, tension, bias);
    vector.y = Interpolate(point0.y, point1.y, point2.y, point3.y, weight, tension, bias);
    vector.z = Interpolate(point0.z, point1.z, point2.z, point3.z, weight, tension, bias);

    return vector;
}

float HermiteCurve::Interpolate(float p0, float p1, float p2, float p3, float t, float tension, float bias) {
    float m0 = (p1 - p0) * (1 + bias) * (1 - tension) / 2
               + (p2 - p1) * (1 - bias) * (1 - tension) / 2;

    float m1 = (p2 - p1) * (1 + bias) * (1 - tension) / 2
               + (p3 - p2) * (1 - bias) * (1 - tension) / 2;

    float t2 = t * t;
    float t3 = t2 * t;

    float h0 = 2 * t3 - 3 * t2 + 1;
    float h1 = t3 - 2 * t2 + t;
    float h2 = t3 - t2;
    float h3 = -2 * t3 + 3 * t2;

    return h0 * p1 + h1 * m0 + h2 * m1 + h3 * p2;
}


glm::vec3 HermiteCurve::InterpolateVec(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, float t) {
    int numberOfPoints = 20;
    // set points of Hermite curve
    glm::vec3 m0 = p0 - p1;
    glm::vec3 m1 = p2 - p1;

    glm::vec3 position;
    for (int i = 0; i < numberOfPoints; i++) {
        t = i / (numberOfPoints - 1.0f);
        position = (2.0f * t * t * t - 3.0f * t * t + 1.0f) * p1
                   + (t * t * t - 2.0f * t * t + t) * m0
                   + (-2.0f * t * t * t + 3.0f * t * t) * p2
                   + (t * t * t - t * t) * m1;
        std::cout << "Hermite at point: " << i << " Pos: " << position.x << " " << position.y << " " << position.z
                  << std::endl;
    }
    return position;
}




