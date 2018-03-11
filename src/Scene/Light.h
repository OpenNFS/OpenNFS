//
// Created by Amrik on 01/03/2018.
//

#ifndef OPENNFS3_LIGHT_H
#define OPENNFS3_LIGHT_H

#include <glm/vec3.hpp>
#include "../nfs_data.h"
#include "Model.h"

class Light : public Model {
public:
    Light(INTPT light_position, long light_type);
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
