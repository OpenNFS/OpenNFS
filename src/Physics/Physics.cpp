//
// Created by Amrik Sadhra on 05/02/2018.
//

#include "Physics.h"

btBoxShape *genCollisionBox(std::vector<glm::vec3> model_vertices) {
    glm::vec3 bottom_left = glm::vec3(model_vertices[0].x, model_vertices[0].y, model_vertices[0].z);
    glm::vec3 top_right = glm::vec3(model_vertices[0].x, model_vertices[0].y, model_vertices[0].z);


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
            btVector3((top_right.x - bottom_left.x) / 2, (top_right.y - bottom_left.y) / 2, (top_right.z - bottom_left.z) / 2));
}