#pragma once

#include <vector>
#include <cstdio>
#include <string>
#include <cstring>
#include <glm/glm.hpp>

bool loadOBJFile(const char * path, std::vector<glm::vec3> & out_vertices, std::vector<glm::vec2> & out_uvs);
