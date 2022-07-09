#pragma once

#include "Model.h"

class TrackModel : public Model {
public:
    TrackModel(std::vector<glm::vec3> &vertices, std::vector<glm::vec3> &normals, std::vector<glm::vec2> &uvs, std::vector<uint32_t> &textureIndices,
               std::vector<uint32_t> &vertexIndices, std::vector<glm::vec4> &shadingData, std::vector<uint32_t> &debugData, glm::vec3 centerPosition);
    TrackModel(std::vector<glm::vec3> &vertices, std::vector<glm::vec3> &normals, std::vector<glm::vec2> &uvs, std::vector<uint32_t> &textureIndices,
               std::vector<uint32_t> &vertexIndices, std::vector<glm::vec4> &shadingData, glm::vec3 centerPosition);
    TrackModel();
    void update() override;
    void destroy() override;
    void render() override;
    bool genBuffers() override;
    std::vector<uint32_t> m_textureIndices;
    std::vector<glm::vec4> m_shadingData;
    std::vector<uint32_t> m_debugData;

private:
    GLuint m_vertexBuffer;
    GLuint m_uvBuffer;
    GLuint m_textureIndexBuffer;
    GLuint m_shadingDataBuffer;
    GLuint m_normalBuffer;
    GLuint m_debugBuffer;
};
