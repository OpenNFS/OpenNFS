//
// Created by Amrik on 03/03/2018.
//

#ifndef OPENNFS3_CAR_H
#define OPENNFS3_CAR_H

#include "Model.h"
#include <iostream>

using namespace std;

class Car : public Model {
public:
    Car(std::string name, int model_id, std::vector<glm::vec3> verts, std::vector<glm::vec2> uvs, std::vector<glm::vec3> norms, std::vector<unsigned int> indices, glm::vec3 center);
    void update() override;
    void destroy() override;
    void render() override;
    bool genBuffers() override;
private:
    GLuint vertexbuffer;
    GLuint uvbuffer;
    GLuint normalbuffer;
    typedef Model super;
};


#endif //OPENNFS3_CAR_H
