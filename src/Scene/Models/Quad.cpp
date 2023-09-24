#include "Quad.h"
#include "../../Util/Utils.h"

Quad::Quad(glm::vec3 position, glm::vec3 colour, float fromX, float fromY, float toX, float toY) :
    super("Quad", std::vector<glm::vec3>(), std::vector<glm::vec2>(), std::vector<glm::vec3>(), std::vector<unsigned int>(), false, position) {
    std::vector<glm::vec3> verts;
    verts.emplace_back(fromX, fromY, 0); // bottom left corner
    verts.emplace_back(toY, fromX, 0);   // top left corner
    verts.emplace_back(toX, toY, 0);     // top right corner
    verts.emplace_back(toX, fromY, 0);   // bottom right corner
    unsigned int indices[] = {0, 1,
                              2,        // first triangle (bottom left - top left - top right)
                              0, 2, 3}; // second triangle (bottom left - top right - bottom right)
    m_uvs.clear();
    m_uvs.emplace_back(1.0f, 1.0f);
    m_uvs.emplace_back(0.0f, 1.0f);
    m_uvs.emplace_back(0.0f, 0.0f);
    m_uvs.emplace_back(1.0f, 1.0f);
    m_uvs.emplace_back(0.0f, 0.0f);
    m_uvs.emplace_back(1.0f, 0.0f);

    m_vertexIndices = std::vector<unsigned int>(indices, indices + sizeof(indices) / sizeof(indices[0]));
    ;
    m_vertices.clear();

    // Unindex data and Fill unused normal buffer
    for (unsigned int m_vertex_index : m_vertexIndices) {
        m_vertices.push_back(verts[m_vertex_index]);
        m_normals.emplace_back(0, 0, 0);
    }

    this->position = position;
    this->colour   = colour;

    enable();
    ASSERT(genBuffers(), "Unable to generate GL Buffers for Quad");
}

void Quad::update() {
    orientation_vec = glm::vec3(-SIMD_HALF_PI, 0, 0);
    orientation     = glm::normalize(glm::quat(orientation_vec));
    RotationMatrix  = glm::toMat4(orientation);
    // Rotate around center
    TranslationMatrix = glm::translate(glm::mat4(1.0), position);
    ModelMatrix       = TranslationMatrix * RotationMatrix;
}

void Quad::destroy() {
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &uvbuffer);
    glDeleteBuffers(1, &normalBuffer);
}

void Quad::render() {
    if (enabled) {
        glBindVertexArray(VertexArrayID);
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei) m_vertices.size());
        glBindVertexArray(0);
    }
}

bool Quad::genBuffers() {
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);
    // Verts
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(glm::vec3), &m_vertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0,         // attribute
                          3,         // size
                          GL_FLOAT,  // type
                          GL_FALSE,  // normalized?
                          0,         // stride
                          (void *) 0 // array buffer offset
    );
    // 1st attribute buffer : Vertices
    glEnableVertexAttribArray(0);
    // UVs
    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, m_uvs.size() * sizeof(glm::vec2), &m_uvs[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1,         // attribute
                          2,         // size
                          GL_FLOAT,  // type
                          GL_FALSE,  // normalized?
                          0,         // stride
                          (void *) 0 // array buffer offset
    );
    // 2nd attribute buffer : normals
    glEnableVertexAttribArray(1);
    // Normals
    glGenBuffers(1, &normalBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    glBufferData(GL_ARRAY_BUFFER, m_normals.size() * sizeof(glm::vec3), &m_normals[0], GL_STATIC_DRAW);
    glVertexAttribPointer(4,         // attribute
                          3,         // size
                          GL_FLOAT,  // type
                          GL_FALSE,  // normalized?
                          0,         // stride
                          (void *) 0 // array buffer offset
    );
    glBindVertexArray(0);
    return true;
}

Quad::Quad() : super("Quad", std::vector<glm::vec3>(), std::vector<glm::vec2>(), std::vector<glm::vec3>(), std::vector<unsigned int>(), false, glm::vec3(0, 0, 0)) {
}
