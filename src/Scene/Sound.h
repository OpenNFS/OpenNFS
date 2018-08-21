//
// Created by Amrik Sadhra on 01/03/2018.
//

#pragma once

#include <glm/vec3.hpp>
#include "../Util/Utils.h"
#include "Model.h"


class Sound : public Model {
public:
    Sound(glm::vec3 sound_position, uint32_t sound_type);
    Sound();

    void update() override;
    void destroy() override;
    void render() override;
    bool genBuffers()override;

    glm::vec3 position{};
    uint32_t type;
private:
    GLuint vertexbuffer;
    GLuint uvbuffer;
    GLuint normalBuffer;
    typedef Model super;
};

