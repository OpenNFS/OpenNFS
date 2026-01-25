#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

#include "../Camera/BaseCamera.h"
#include "Shaders/SkidMarkShader.h"

namespace OpenNFS {
    // Forward declaration
    struct VehicleTrails;

    // Vertex data for skid mark triangle strip
    struct SkidMarkVertex {
        glm::vec3 position;
        glm::vec2 uv;
        glm::vec3 normal;
        float age;
        float intensity;
        uint32_t surfaceType;
    };

    class SkidMarkRenderer {
      public:
        SkidMarkRenderer();
        ~SkidMarkRenderer();

        void Render(BaseCamera const &camera, std::vector<VehicleTrails> const &allTrails,
                    float lifetime, float fadeStart);

        // Delete copy operations (we manage OpenGL resources)
        SkidMarkRenderer(SkidMarkRenderer const &) = delete;
        SkidMarkRenderer &operator=(SkidMarkRenderer const &) = delete;

      private:
        void InitBuffers();
        void BuildVertexData(std::vector<VehicleTrails> const &allTrails);
        void GenerateDefaultTexture();

        SkidMarkShader m_shader;
        GLuint m_vao{0};
        GLuint m_vbo{0};
        GLuint m_defaultTexture{0};

        std::vector<SkidMarkVertex> m_vertices;

        // Max vertices we can render
        static constexpr size_t MAX_VERTICES = 8192;
    };
} // namespace OpenNFS
