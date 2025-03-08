#pragma once

#include "BaseShader.h"

#include <glm/detail/type_mat4x4.hpp>

namespace OpenNFS {
    class UIShader : public BaseShader {
    public:
        explicit UIShader();

        void loadLayer(GLint layer);

        void loadColour(glm::vec3 colour);

        void loadProjectionMatrix(const glm::mat4 &projection);

        void loadUITexture(GLuint textureID);

    protected:
        void bindAttributes() override;

        void getAllUniformLocations() override;

        void customCleanup() override;

        GLint menuTextureSamplerLocation;
        GLint projectionMatrixLocation;
        GLint layerLocation;
        GLint colourLocation;
    };
}
