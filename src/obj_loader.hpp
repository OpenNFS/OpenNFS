//
// Created by Amrik Sadhra on 08/10/2017.
//

#ifndef FCE_TO_OBJ_OBJ_LOADER_HPP_H
#define FCE_TO_OBJ_OBJ_LOADER_HPP_H

#include <vector>
#include <glm/vec3.hpp>
#include <glm/detail/type_vec2.hpp>
#include <string>


bool loadOBJ(
        const char * path,
        std::vector<glm::vec3> & out_vertices,
        std::vector<glm::vec2> & out_uvs,
        std::vector<glm::vec3> & out_normals
);
#endif //FCE_TO_OBJ_OBJ_LOADER_HPP_H
