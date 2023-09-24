#pragma once

#include "BaseShader.h"
#include <sstream>
#include <glm/detail/type_mat4x4.hpp>
#include <Models/BaseLight.h>

#include "../Config.h"
#include "../Util/ImageLoader.h"
#include "../Scene/Models/Model.h"
#include "../Physics/Car.h"
#include "../../shaders/ShaderPreamble.h"

class CarShader : public BaseShader {
public:
    explicit CarShader();
    void loadCarColor(glm::vec3 color);
    void loadCarTexture(GLuint textureID);
    void loadLights(const std::vector<shared_ptr<BaseLight>> &lights);
    void loadSpecular(float damper, float reflectivity, float env_reflectivity);
    void loadProjectionViewMatrices(const glm::mat4 &projection, const glm::mat4 &view);
    void loadTransformationMatrix(const glm::mat4 &transformation);
    void bindTextureArray(GLuint textureArrayID);
    void setMultiTextured(bool multiTextured);
    void setPolyFlagged(bool polyFlagged);
    void loadEnvironmentMapTexture();

protected:
    void bindAttributes() override;
    void getAllUniformLocations() override;
    void customCleanup() override;
    void loadEnvMapTextureData();

    GLint transformationMatrixLocation;
    GLint projectionMatrixLocation;
    GLint viewMatrixLocation;
    GLint envMapTextureLocation;
    GLint carTextureLocation;
    GLint colourLocation;
    GLint lightPositionLocation[MAX_CAR_CONTRIB_LIGHTS];
    GLint lightColourLocation[MAX_CAR_CONTRIB_LIGHTS];
    GLint attenuationLocation[MAX_CAR_CONTRIB_LIGHTS];
    GLint shineDamperLocation;
    GLint reflectivityLocation;
    GLint envReflectivityLocation;
    GLint carTextureArrayLocation; // Multitextured
    GLint isMultiTexturedLocation; // Multitextured
    GLint hasPolyFlagsLocation;    // PolyFlagged

    GLuint envMapTextureID = 0;

    typedef BaseShader super;
};
