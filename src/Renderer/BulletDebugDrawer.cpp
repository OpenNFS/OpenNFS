#include "BulletDebugDrawer.h"

BulletDebugDrawer::BulletDebugDrawer() {
    // Configure VAO for line render data
    glGenVertexArrays(1, &m_lineVAO);
    glBindVertexArray(m_lineVAO);

    // Generate two VBO handles
    glGenBuffers(2, m_lineVertexBuffers);

    // VBO for line vertices
    glBindBuffer(GL_ARRAY_BUFFER, m_lineVertexBuffers[LineVBO::VERTEX]);
    glBufferData(GL_ARRAY_BUFFER, INITIAL_LINE_BUF_SIZE * sizeof(BulletLine), nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    // And a separate VBO for line colours
    glBindBuffer(GL_ARRAY_BUFFER, m_lineVertexBuffers[LineVBO::COLOUR]);
    glBufferData(GL_ARRAY_BUFFER, INITIAL_LINE_BUF_SIZE * sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(1);

    // Reset state
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void BulletDebugDrawer::drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &color) {
    if (m_debugLines.size() < MAX_NUM_LINES) {
        m_debugLines.emplace_back(BulletLine(from, to));
        m_debugLineColours.emplace_back(Utils::bulletToGlm(color));
        m_debugLineColours.emplace_back(Utils::bulletToGlm(color));
    }
}

void BulletDebugDrawer::Render(const BaseCamera &camera) {
    // Activate corresponding render state
    m_bulletShader.use();
    m_bulletShader.loadProjectionViewMatrix(camera.projectionMatrix * camera.viewMatrix);

    // Update content of VBO memory
    glBindVertexArray(m_lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_lineVertexBuffers[LineVBO::VERTEX]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, m_debugLines.size() * sizeof(BulletLine), &m_debugLines[0]);

    glBindBuffer(GL_ARRAY_BUFFER, m_lineVertexBuffers[LineVBO::COLOUR]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, m_debugLineColours.size() * sizeof(glm::vec3), &m_debugLineColours[0]);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Render line
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(m_debugLines.size() * 2));

    // Reset state
    glBindVertexArray(0);
    m_bulletShader.unbind();

    m_debugLines.clear();
    m_debugLineColours.clear();
}
