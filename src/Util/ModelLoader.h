#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>

namespace OpenNFS {
    struct ModelData {
        std::vector<glm::vec3> m_vertices;
        std::vector<glm::vec3> m_normals;
        std::vector<glm::vec2> m_uvs;
        std::vector<unsigned int> m_indices;
    };

    class ModelLoader {
    public:
        static bool LoadObj(const std::string& path, std::vector<ModelData>& models, float scale = 1.0f);
    };
}
