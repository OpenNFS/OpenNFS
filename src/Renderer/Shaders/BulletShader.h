#pragma once

#include "BaseShader.h"

#include <glm/detail/type_mat4x4.hpp>

namespace OpenNFS {
    class BulletShader : public BaseShader {
    public:
        explicit BulletShader();

        void loadProjectionViewMatrix(const glm::mat4 &projectionViewMatrix) const;

    protected:
        void bindAttributes() override;

        void getAllUniformLocations() override;

        void customCleanup() override;

        GLint projectionViewMatrixLocation;
    };
}
