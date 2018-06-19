//
// Created by Amrik on 01/03/2018.
//

#ifndef OPENNFS3_LIGHT_H
#define OPENNFS3_LIGHT_H

#include <glm/vec3.hpp>
#include "Model.h"
#include "../nfs_data.h"

struct INTPT {
    uint32_t x, z, y;
};


class Light : public Model {
public:
    Light(INTPT light_position, uint32_t light_type);
    Light(glm::vec3 light_position, glm::vec3 light_colour);

    void update() override;
    void destroy() override;
    void render() override;
    bool genBuffers()override;

    glm::vec3 position{};
    glm::vec3 colour;
    long type;
private:
    GLuint vertexbuffer;
    GLuint uvbuffer;
    GLuint normalBuffer;
    typedef Model super;
};


#endif //OPENNFS3_LIGHT_H
