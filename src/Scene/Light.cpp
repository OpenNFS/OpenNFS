//
// Created by Amrik on 01/03/2018.
//

#include "Light.h"

Light::Light(glm::vec3 light_position, glm::vec3 light_colour){
    position= light_position;
    type = 1;
    colour = light_colour;
}

Light::Light(glm::vec3 light_position, long light_type){
    position= light_position;
    type = light_type;
    colour = glm::vec3(1.0,1.0,1.0);
}
Light::Light(INTPT light_position, long light_type){
        position.x = static_cast<float>(light_position.x / 65536.0);
        position.y = static_cast<float>(light_position.y / 65536.0);
        position.z = static_cast<float>(light_position.z / 65536.0);
        type = light_type;
        colour = glm::vec3(0.8, 0.7,0.5);
};