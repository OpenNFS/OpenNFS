//
// Created by Amrik on 03/03/2018.
//

#ifndef OPENNFS3_TRACK_H
#define OPENNFS3_TRACK_H

#include "Model.h"

class Track : public Model {
public:
    Track(std::string name, int model_id, std::vector<glm::vec3> verts, std::vector<glm::vec2> uvs, std::vector<unsigned int> texture_indices, std::vector<unsigned int> indices, std::vector<short> tex_ids, std::vector<glm::vec4> shading_data, glm::vec3 center_position);
    Track(std::string name, int model_id, std::vector<glm::vec3> verts, std::vector<glm::vec3> norms, std::vector<glm::vec2> uvs, std::vector<unsigned int> texture_indices, std::vector<unsigned int> indices, std::vector<short> tex_ids, std::vector<glm::vec4> shading_data, glm::vec3 center_position);
    void update() override;
    void destroy() override;
    void render() override;
    bool genBuffers()override;
    std::vector<unsigned int> m_texture_indices;
    std::vector<short> texture_ids;
private:
    GLuint vertexbuffer;
    GLuint uvbuffer;
    GLuint textureIndexBuffer;
    GLuint shadingBuffer;
    GLuint normalBuffer;
    std::vector<glm::vec4> shadingData;
    typedef Model super;

    btCollisionShape *GenCollisionData();
};


#endif //OPENNFS3_TRACK_H





