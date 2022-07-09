#pragma once

#include "Model.h"

class CarColour {
public:
    std::string colourName;
    glm::vec3 colour;
    CarColour(const std::string &colourName, glm::vec3 colour) {
        this->colourName = colourName;
        this->colour     = colour;
    }
};

class Dummy {
public:
    std::string name;
    glm::vec3 position;
    Dummy(const char *dummyName, glm::vec3 position) {
        std::string nameTemp(dummyName);

        this->name     = nameTemp;
        this->position = position;
    }
};

class CarModel : public Model {
public:
    // Test
    CarModel(std::string name,
             std::vector<glm::vec3>
               verts,
             std::vector<glm::vec2>
               uvs,
             std::vector<unsigned int>
               texture_indices,
             std::vector<uint32_t>
               test,
             std::vector<glm::vec3>
               norms,
             std::vector<unsigned int>
               indices,
             glm::vec3 center_position,
             float specular_damper,
             float specular_reflectivity,
             float env_reflectivity);
    // TODO: Alter all these constructors to const reference
    // Multitextured Cars (NFS2)
    CarModel(std::string name,
             std::vector<glm::vec3>
               verts,
             std::vector<glm::vec2>
               uvs,
             std::vector<unsigned int>
               texture_indices,
             std::vector<glm::vec3>
               norms,
             std::vector<unsigned int>
               indices,
             glm::vec3 center_position,
             float specular_damper,
             float specular_reflectivity,
             float env_reflectivity);
    // Cars with Per-Polygon Flags (NFS4)
    CarModel(std::string name,
             std::vector<glm::vec3>
               verts,
             std::vector<glm::vec2>
               uvs,
             std::vector<glm::vec3>
               norms,
             std::vector<unsigned int>
               indices,
             std::vector<uint32_t>
               poly_flags,
             glm::vec3 center_position,
             float specular_damper,
             float specular_reflectivity,
             float env_reflectivity);
    // Vanilla Cars (NFS3)
    CarModel(std::string name,
             std::vector<glm::vec3>
               verts,
             std::vector<glm::vec2>
               uvs,
             std::vector<glm::vec3>
               norms,
             std::vector<unsigned int>
               indices,
             glm::vec3 center_position,
             float specular_damper,
             float specular_reflectivity,
             float env_reflectivity);
    CarModel();
    void update() override;
    void destroy() override;
    void render() override;
    bool genBuffers() override;
    // Car Display params
    float specularDamper;
    float specularReflectivity;
    float envReflectivity;

    // Multitextured Car
    bool isMultiTextured = false;
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

class CarData {
public:
    CarData() : carName("Unset"){};
    std::string carName;
    std::vector<Dummy> dummies;
    std::vector<CarColour> colours;
    std::vector<CarModel> meshes;
};