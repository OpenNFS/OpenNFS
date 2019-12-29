#pragma once

#include <LinearMath/btIDebugDraw.h>
#include <glm/detail/type_mat4x4.hpp>

#include "../Camera/Camera.h"
#include "../Shaders/BulletShader.h"
#include "../Util/Utils.h"
#include "../Util/Logger.h"

const uint32_t INITIAL_LINE_BUF_SIZE = 1024 * 1000;

class BulletLine {
    glm::vec3 from;
    glm::vec3 to;
public:
    BulletLine(const btVector3 &from, const btVector3 &to)
    {
        this->from = Utils::bulletToGlm(from);
        this->to = Utils::bulletToGlm(to);
    }
};


class BulletDebugDrawer : public btIDebugDraw {
public:
    BulletDebugDrawer();

    void Render(const std::shared_ptr<Camera> &camera);

    void drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &color) override;

    void drawContactPoint(const btVector3 &, const btVector3 &, btScalar, int, const btVector3 &) override {}

    void reportErrorWarning(const char *warningString) override
    {
        LOG(WARNING) << warningString;
    }

    void draw3dText(const btVector3 &location, const char *textString) override
    {
        // TODO: Bring in MenuRenderer
        LOG(WARNING) << textString;
    }

    void setDebugMode(int p) override {
        m = p;
    }

    int getDebugMode(void) const override { return 3; }

    int m{};

private:
    // OpenGL data
    GLuint m_lineVAO{}, m_lineVertexVBO{}, m_lineColourVBO{};
    // Render shaders
    BulletShader m_bulletShader;
    std::vector<BulletLine> m_debugLines;
    std::vector<glm::vec3> m_debugLineColours;
};


