//
// Created by Amrik Sadhra on 20/07/2018.
//

#pragma once

#include <cmath>
#include <glm/vec3.hpp>
#include <iostream>
#include <vector>

class HermiteCurve {
  public:
    HermiteCurve(std::vector<glm::vec3> points, float tension, float bias);

    HermiteCurve();

    glm::vec3 getPointAt(float t);
    float Interpolate(float p0, float p1, float p2, float p3, float t, float tension, float bias);
    glm::vec3 Interpolate(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, float t, float tension, float bias);
    std::vector<glm::vec3> points;

  private:
    float tension;
    float bias;
};
