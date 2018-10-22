//
// Created by Amrik Sadhra on 01/03/2018.
//

#pragma once

#include <glm/vec3.hpp>
#include "Model.h"


class Light : public Model {
public:
    Light(glm::vec3 light_position, glm::vec4 light_colour, int light_type, int unknown_1, int unknown_2, int unknown_3, float unknown_4);
    Light();

    void update() override;
    void destroy() override;
    void render() override;
    bool genBuffers()override;

    glm::vec3 attenuation = glm::vec3(2.0f, 0.0f, 0.1f);
    glm::vec4 colour;
    long type;

    // Point light is looking at
    glm::vec3 lookAt = glm::vec3(0.f,0.f,0.f);
    glm::mat4 ViewMatrix; // For shadow mapping Depth

    // NFS3 and 4 light data stored in TR.ini [track glows]
    int unknown1, unknown2, unknown3;
    float unknown4;
private:
    GLuint vertexbuffer;
    GLuint uvbuffer;
    GLuint normalBuffer;
    typedef Model super;
};

