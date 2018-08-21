//
// Created by Amrik Sadhra on 03/03/2018.
//

#ifndef OPENNFS3_CARMODEL_H
#define OPENNFS3_CARMODEL_H

#include "Model.h"

using namespace std;

class CarModel : public Model {
public:
    CarModel(std::string name, std::vector<glm::vec3> verts, std::vector<glm::vec2> uvs, std::vector<unsigned int> texture_indices, std::vector<glm::vec3> norms, std::vector<unsigned int> indices, std::vector<unsigned int> tex_ids, glm::vec3 center_position, float specular_damper, float specular_reflectivity, float env_reflectivity);
    CarModel(std::string name, std::vector<glm::vec3> verts, std::vector<glm::vec2> uvs, std::vector<glm::vec3> norms, std::vector<unsigned int> indices, glm::vec3 center_position, float specular_damper, float specular_reflectivity, float env_reflectivity);
    CarModel();
    void update() override;
    void destroy() override;
    void render() override;
    bool genBuffers() override;
    float specularDamper;
    float specularReflectivity;
    float envReflectivity;
    // Multitextured
    std::vector<unsigned int> texture_ids;
    private:
    GLuint vertexbuffer;
    GLuint uvbuffer;
    GLuint normalbuffer;
    // Multitextured Car
    GLuint textureIndexBuffer;
    std::vector<unsigned int> m_texture_indices;
    typedef Model super;
};


#endif //OPENNFS3_CARMODEL_H
