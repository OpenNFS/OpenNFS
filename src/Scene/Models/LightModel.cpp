#include "LightModel.h"

#include "../../../../src/Util/Utils.h"

LightModel::LightModel() : Model("Light", std::vector<glm::vec3>(), std::vector<glm::vec2>(), std::vector<glm::vec3>(), std::vector<unsigned int>(), false, glm::vec3()) {
    m_vertexIndices = {0, 1,
                       2,        // first triangle (bottom left - top left - top right)
                       0, 2, 3}; // second triangle (bottom left - top right - bottom right)

    // Unindex data and Fill unused normal buffer
    for (auto &vertexIndex : m_vertexIndices) {
        m_vertices.push_back(verts[vertexIndex]);
    }

    m_uvs = {glm::vec2(1.0f, 1.0f), glm::vec2(0.0f, 1.0f), glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 0.0f)};

    enable();

    ASSERT(genBuffers(), "Unable to generate GL Buffers for Light");
}

void LightModel::destroy() {
    glDeleteBuffers(LightVBO::Length, m_lightVertexBuffers);
}

void LightModel::render() {
    if (enabled) {
        glBindVertexArray(VertexArrayID);
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei) m_vertices.size());
        glBindVertexArray(0);
    }
}

bool LightModel::genBuffers() {
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);
    glGenBuffers(LightVBO::Length, m_lightVertexBuffers);

    // Verts
    glBindBuffer(GL_ARRAY_BUFFER, m_lightVertexBuffers[LightVBO::VERTEX]);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(glm::vec3), &m_vertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    // UVs
    glBindBuffer(GL_ARRAY_BUFFER, m_lightVertexBuffers[LightVBO::UV]);
    glBufferData(GL_ARRAY_BUFFER, m_uvs.size() * sizeof(glm::vec2), &m_uvs[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    return true;
}