#pragma once

#include "Model.h"

class TrackModel : public Model
{
public:
    TrackModel(std::vector<glm::vec3> verts, std::vector<glm::vec3> norms, std::vector<glm::vec2> uvs, std::vector<unsigned int> texture_indices, std::vector<unsigned int> indices,
               std::vector<glm::vec4> shading_data, std::vector<uint32_t> debug_data, glm::vec3 center_position);
    TrackModel(std::vector<glm::vec3> verts, std::vector<glm::vec2> uvs, std::vector<unsigned int> texture_indices, std::vector<unsigned int> indices,
               std::vector<glm::vec4> shading_data, glm::vec3 center_position);
    TrackModel(std::vector<glm::vec3> verts, std::vector<glm::vec3> norms, std::vector<glm::vec2> uvs, std::vector<unsigned int> texture_indices, std::vector<unsigned int> indices,
               std::vector<glm::vec4> shading_data, glm::vec3 center_position);
    TrackModel();
    void update() override;
    void destroy() override;
    void render() override;
    bool genBuffers() override;
    std::vector<unsigned int> m_texture_indices;
    std::vector<glm::vec4> m_shading_data;
    std::vector<uint32_t> m_debug_data;

private:
    GLuint vertexbuffer;
    GLuint uvbuffer;
    GLuint textureIndexBuffer;
    GLuint shadingBuffer;
    GLuint normalBuffer;
    GLuint debugBuffer;
    typedef Model super;
};
