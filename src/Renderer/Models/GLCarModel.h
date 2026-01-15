#pragma once

#include "GLModel.h"
#include <Entities/CarGeometry.h>

namespace OpenNFS {
    class GLCarModel : public GLModel, public LibOpenNFS::CarGeometry {
      public:
        GLCarModel(CarGeometry const &geometry, float specular_damper, float specular_reflectivity, float env_reflectivity);
        explicit GLCarModel(CarGeometry const &geometry);
        GLCarModel() : GLModel() {};
        ~GLCarModel() override;

        // Non-copyable (OpenGL resources)
        GLCarModel(GLCarModel const &) = delete;
        GLCarModel &operator=(GLCarModel const &) = delete;
        // Movable
        GLCarModel(GLCarModel &&other) noexcept;
        GLCarModel &operator=(GLCarModel &&other) noexcept;

        bool GenBuffers() override;
        void UpdateMatrices() override;
        void Render() override;

        // Car Display params
        float specularDamper{0.2f};
        float specularReflectivity{0.02f};
        float envReflectivity{0.4f};

      private:
        GLuint vertexBuffer{};
        GLuint uvBuffer{};
        GLuint normalBuffer{};

        // Multitextured Car
        GLuint textureIndexBuffer{};
        // NFS4 Car
        GLuint polyFlagBuffer{};
    };
} // namespace OpenNFS
