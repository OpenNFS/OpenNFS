#pragma once

#include <LinearMath/btIDebugDraw.h>
#include <glm/detail/type_mat4x4.hpp>

#include "../Camera/BaseCamera.h"
#include "../Shaders/BulletShader.h"
#include "../Util/Utils.h"
#include "../Util/Logger.h"

class BulletLine {
    glm::vec3 from;
    glm::vec3 to;

public:
    BulletLine(const btVector3 &from, const btVector3 &to) {
        this->from = ::Utils::bulletToGlm(from);
        this->to   = ::Utils::bulletToGlm(to);
    }
};

const uint32_t MAX_NUM_LINES         = 10000;
const uint32_t INITIAL_LINE_BUF_SIZE = sizeof(BulletLine) * MAX_NUM_LINES;

class BulletDebugDrawer : public btIDebugDraw {
public:
    BulletDebugDrawer();

    void Render(const BaseCamera &camera);

    void drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &color) override;

    void drawContactPoint(const btVector3 &, const btVector3 &, btScalar, int, const btVector3 &) override {
    }

    void reportErrorWarning(const char *warningString) override {
        LOG(WARNING) << warningString;
    }

    void draw3dText(const btVector3 &location, const char *textString) override {
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
    enum LineVBO : uint8_t { VERTEX = 0, COLOUR, Length };
    GLuint m_lineVertexBuffers[LineVBO::Length]{};
    GLuint m_lineVAO{};
    // Render shaders
    BulletShader m_bulletShader;
    std::vector<BulletLine> m_debugLines;
    std::vector<glm::vec3> m_debugLineColours;
};
