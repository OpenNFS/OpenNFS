#pragma once

#include <LinearMath/btIDebugDraw.h>
#include <glm/detail/type_mat4x4.hpp>

#include "../Camera/BaseCamera.h"
#include "../Util/Logger.h"
#include "../Util/Utils.h"
#include "Shaders/BulletShader.h"

namespace OpenNFS {
    class BulletLine {
        glm::vec3 from;
        glm::vec3 to;

      public:
        BulletLine(btVector3 const &from, btVector3 const &to) {
            this->from = Utils::bulletToGlm(from);
            this->to = Utils::bulletToGlm(to);
        }
    };

    uint32_t const MAX_NUM_LINES = 100000;
    uint32_t const INITIAL_LINE_BUF_SIZE = sizeof(BulletLine) * MAX_NUM_LINES;

    class BulletDebugDrawer : public btIDebugDraw {
      public:
        BulletDebugDrawer();

        void Render(BaseCamera const &camera);

        void drawLine(btVector3 const &from, btVector3 const &to, btVector3 const &color) override;

        void drawContactPoint(btVector3 const &, btVector3 const &, btScalar, int, btVector3 const &) override {
        }

        void reportErrorWarning(char const *warningString) override {
            LOG(WARNING) << warningString;
        }

        void draw3dText(btVector3 const &location, char const *textString) override {
            // TODO: Bring in MenuRenderer
            LOG(WARNING) << textString;
        }

        void setDebugMode(int p) override {
            m = p;
        }

        int getDebugMode(void) const override {
            return 3;
        }

        int m{};

      private:
        // OpenGL data
        enum LineVBO : uint8_t {
            VERTEX = 0,
            COLOUR,
            Length
        };
        GLuint m_lineVertexBuffers[LineVBO::Length]{};
        GLuint m_lineVAO{};
        // Render shaders
        BulletShader m_bulletShader;
        std::vector<BulletLine> m_debugLines;
        std::vector<glm::vec3> m_debugLineColours;
    };

} // namespace OpenNFS
