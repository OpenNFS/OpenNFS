//
// Created by Amrik Sadhra on 05/02/2018.
//

#include "physics.h"

btBoxShape *genCollisionBox(std::vector<glm::vec3> model_vertices) {
    glm::vec3 bottom_left = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 top_right = glm::vec3(0.0f, 0.0f, 0.0f);


    for (auto &vertex : model_vertices) {
        if (vertex.x < bottom_left.x) {
            bottom_left.x = vertex.x;
        }
        if (vertex.y < bottom_left.y) {
            bottom_left.y = vertex.y;
        }
        if (vertex.z < bottom_left.z) {
            bottom_left.z = vertex.z;
        }
        if (vertex.x > top_right.x) {
            top_right.x = vertex.x;
        }
        if (vertex.y > top_right.y) {
            top_right.y = vertex.y;
        }
        if (vertex.z > top_right.z) {
            top_right.z = vertex.z;
        }
    }

    return new btBoxShape(
            btVector3(top_right.x - bottom_left.x, top_right.y - bottom_left.y, top_right.z - bottom_left.z));
}