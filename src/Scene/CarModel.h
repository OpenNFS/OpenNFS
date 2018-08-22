//
// Created by Amrik Sadhra on 03/03/2018.
//

#pragma once

#include "Model.h"

using namespace std;

class CarModel : public Model {
public:
    // TODO: Alter all these constructors to const reference
    // Multitextured Cars (NFS2)
    CarModel(std::string name, std::vector<glm::vec3> verts, std::vector<glm::vec2> uvs, std::vector<unsigned int> texture_indices, std::vector<glm::vec3> norms, std::vector<unsigned int> indices, std::vector<unsigned int> tex_ids, glm::vec3 center_position, float specular_damper, float specular_reflectivity, float env_reflectivity);
    // Cars with Per-Polygon Flags (NFS4)
    CarModel(std::string name, std::vector<glm::vec3> verts, std::vector<glm::vec2> uvs, std::vector<glm::vec3> norms, std::vector<unsigned int> indices, std::vector<uint32_t> poly_flags, glm::vec3 center_position, float specular_damper, float specular_reflectivity, float env_reflectivity);
    // Vanilla Cars (NFS3)
    CarModel(std::string name, std::vector<glm::vec3> verts, std::vector<glm::vec2> uvs, std::vector<glm::vec3> norms, std::vector<unsigned int> indices, glm::vec3 center_position, float specular_damper, float specular_reflectivity, float env_reflectivity);
    CarModel();
    void update() override;
    void destroy() override;
    void render() override;
    bool genBuffers() override;
    float specularDamper;
    float specularReflectivity;
    float envReflectivity;

    // Multitextured Car
    std::vector<unsigned int> texture_ids;
    bool isMultiTextured = false; // Avoid checking texture_ids.size() every Shader bind
    // NFS4 Car
    std::vector<uint32_t> m_polygon_flags;
    bool hasPolyFlags = false; // Avoid checking polygon_flags.size() every Shader bind
private:
    GLuint vertexBuffer;
    GLuint uvBuffer;
    GLuint normalBuffer;

    // Multitextured Car
    GLuint textureIndexBuffer;
    std::vector<unsigned int> m_texture_indices;
    // NFS4 Car
    GLuint polyFlagBuffer;

    typedef Model super;
};
