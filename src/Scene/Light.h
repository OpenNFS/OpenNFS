//
// Created by Amrik on 01/03/2018.
//

#ifndef OPENNFS3_LIGHT_H
#define OPENNFS3_LIGHT_H

#include <glm/vec3.hpp>
#include "Model.h"
#include "../Loaders/nfs3_loader.h"

class Light : public Model {
public:
    Light(INTPT light_position, uint32_t light_type);
    Light(glm::vec3 light_position, glm::vec3 light_colour);

    Light();

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
