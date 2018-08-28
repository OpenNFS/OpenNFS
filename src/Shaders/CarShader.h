//
// Created by Amrik Sadhra on 01/03/2018.
//

#pragma once

#include "BaseShader.h"
#include <sstream>
#include <glm/detail/type_mat4x4.hpp>
#include "../Config.h"
#include "../Util/Utils.h"
#include "../Scene/Model.h"
#include "../../include/TGALoader/TGALoader.h"
#include "../Scene/Light.h"
#include "../Physics/Car.h"


class CarShader : public BaseShader {
public:
    explicit CarShader(shared_ptr<Car> current_car);
    void loadCarColor(glm::vec3 color);
    void loadCarTexture();
    void loadLight(Light light);
    void loadSpecular(float damper, float reflectivity, float env_reflectivity);
    void loadMatrices(const glm::mat4 &projection, const glm::mat4 &view, const glm::mat4 &transformation);
    void bindCarTextures(const CarModel &car_model, std::map<unsigned int, GLuint> gl_id_map);
    void setMultiTextured(bool multiTextured);
    void setPolyFlagged(bool polyFlagged);
    void load_tga_texture();
protected:
    void bindAttributes() override;
    void getAllUniformLocations() override;
    void customCleanup() override;
    shared_ptr<Car> car;
    GLint transformationMatrixLocation;
    GLint projectionMatrixLocation;
    GLint viewMatrixLocation;
    GLint envMapTextureLocation;
    GLint carTextureLocation;
    GLint colourLocation;
    GLint lightPositionLocation;
    GLint lightColourLocation;
    GLint shineDamperLocation;
    GLint reflectivityLocation;
    GLint envReflectivityLocation;
    GLint isMultiTexturedLocation;
    GLint hasPolyFlagsLocation;
    GLuint textureID;
    GLuint envMapTextureID;
    // Multitextured
    GLint CarTexturesID;

    typedef BaseShader super;

    void loadEnvironmentMapTexture();
    void LoadEnvMapTexture();
};


