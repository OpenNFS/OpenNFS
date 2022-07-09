#include "CarModel.h"
#include "../../Util/Utils.h"

CarModel::CarModel(std::string name,
                   std::vector<glm::vec3>
                     verts,
                   std::vector<glm::vec2>
                     uvs,
                   std::vector<unsigned int>
                     texture_indices,
                   std::vector<uint32_t>
                     test,
                   std::vector<glm::vec3>
                     norms,
                   std::vector<unsigned int>
                     indices,
                   glm::vec3 center_position,
                   float specular_damper,
                   float specular_reflectivity,
                   float env_reflectivity) :
    super(name, verts, uvs, norms, indices, true, center_position) {
    m_texture_indices = texture_indices;
    isMultiTextured   = true;
    // Fill the unused buffer with data
    m_polygon_flags = test;
    // Can't call basic constructor as genBuffers() call would run before m_textureIndices was available
    specularDamper       = specular_damper;
    specularReflectivity = specular_reflectivity;
    envReflectivity      = env_reflectivity;
    m_normals.clear();
    for (unsigned int m_vertex_index : m_vertexIndices) {
        m_normals.push_back(norms[m_vertex_index]);
    }

    // Gen VBOs, add to Bullet Physics
    ASSERT(genBuffers(), "Unable to generate GL Buffers for Car Model ");
}

CarModel::CarModel(std::string name,
                   std::vector<glm::vec3>
                     verts,
                   std::vector<glm::vec2>
                     uvs,
                   std::vector<unsigned int>
                     texture_indices,
                   std::vector<glm::vec3>
                     norms,
                   std::vector<unsigned int>
                     indices,
                   glm::vec3 center_position,
                   float specular_damper,
                   float specular_reflectivity,
                   float env_reflectivity) :
    super(name, verts, uvs, norms, indices, true, center_position) {
    m_texture_indices = texture_indices;
    isMultiTextured   = true;
    // Fill the unused buffer with data
    for (int i = 0; i < m_texture_indices.size(); ++i) {
        m_polygon_flags.emplace_back(0);
    }
    // Can't call basic constructor as genBuffers() call would run before m_textureIndices was available
    specularDamper       = specular_damper;
    specularReflectivity = specular_reflectivity;
    envReflectivity      = env_reflectivity;
    m_normals            = norms;

    // Gen VBOs, add to Bullet Physics
    ASSERT(genBuffers(), "Unable to generate GL Buffers for Car Model ");
}

CarModel::CarModel(std::string name,
                   std::vector<glm::vec3>
                     verts,
                   std::vector<glm::vec2>
                     uvs,
                   std::vector<glm::vec3>
                     norms,
                   std::vector<unsigned int>
                     indices,
                   std::vector<uint32_t>
                     poly_flags,
                   glm::vec3 center_position,
                   float specular_damper,
                   float specular_reflectivity,
                   float env_reflectivity) :
    super(name, verts, uvs, norms, indices, true, center_position) {
    m_polygon_flags = poly_flags;
    hasPolyFlags    = true;
    // Fill the unused buffer with data
    for (int i = 0; i < m_vertexIndices.size(); ++i) {
        m_texture_indices.emplace_back(0);
    }
    // TODO: Refactor to allow an inline constructor call
    // Can't call basic constructor as genBuffers() call would run before m_polygon_flags was available
    specularDamper       = specular_damper;
    specularReflectivity = specular_reflectivity;
    envReflectivity      = env_reflectivity;
    m_normals.clear();
    for (unsigned int m_vertex_index : m_vertexIndices) {
        m_normals.push_back(norms[m_vertex_index]);
    }

    // Gen VBOs, add to Bullet Physics
    ASSERT(genBuffers(), "Unable to generate GL Buffers for Car Model ");
}

CarModel::CarModel(std::string name,
                   std::vector<glm::vec3>
                     verts,
                   std::vector<glm::vec2>
                     uvs,
                   std::vector<glm::vec3>
                     norms,
                   std::vector<unsigned int>
                     indices,
                   glm::vec3 center_position,
                   float specular_damper,
                   float specular_reflectivity,
                   float env_reflectivity) :
    super(name, verts, uvs, norms, indices, false, center_position) {
    // Fill the unused buffer with data
    for (int i = 0; i < m_normals.size(); ++i) {
        m_texture_indices.emplace_back(0);
    }
    for (int i = 0; i < m_texture_indices.size(); ++i) {
        m_polygon_flags.emplace_back(0);
    }
    specularDamper       = specular_damper;
    specularReflectivity = specular_reflectivity;
    envReflectivity      = env_reflectivity;
    m_normals.clear();
    for (unsigned int m_vertex_index : m_vertexIndices) {
        m_normals.push_back(norms[m_vertex_index]);
    }

    // Gen VBOs, add to Bullet Physics
    ASSERT(genBuffers(), "Unable to generate GL Buffers for Car Model ");
}

CarModel::CarModel() : super("CarModel", std::vector<glm::vec3>(), std::vector<glm::vec2>(), std::vector<glm::vec3>(), std::vector<unsigned int>(), false, glm::vec3(0, 0, 0)) {
}

void CarModel::update() {
    RotationMatrix    = glm::toMat4(orientation);
    TranslationMatrix = glm::translate(glm::mat4(1.0), position);
    ModelMatrix       = TranslationMatrix * RotationMatrix;
}

void CarModel::destroy() {
    if (!Config::get().vulkanRender) {
        glDeleteBuffers(1, &vertexBuffer);
        glDeleteBuffers(1, &uvBuffer);
        glDeleteBuffers(1, &normalBuffer);
        glDeleteBuffers(1, &textureIndexBuffer);
        glDeleteBuffers(1, &polyFlagBuffer);
    }
}

void CarModel::render() {
    if (enabled) {
        glBindVertexArray(VertexArrayID);
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei) m_vertices.size());
        glBindVertexArray(0);
    }
}

bool CarModel::genBuffers() {
    if (Config::get().vulkanRender)
        return true;

    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // Verts
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
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
    glGenBuffers(1, &uvBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
    glBufferData(GL_ARRAY_BUFFER, m_uvs.size() * sizeof(glm::vec2), &m_uvs[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1,         // attribute
                          2,         // size
                          GL_FLOAT,  // type
                          GL_FALSE,  // normalized?
                          0,         // stride
                          (void *) 0 // array buffer offset
    );
    // 2nd attribute buffer : UVs
    glEnableVertexAttribArray(1);
    // Normals
    glGenBuffers(1, &normalBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    glBufferData(GL_ARRAY_BUFFER, m_normals.size() * sizeof(glm::vec3), &m_normals[0], GL_STATIC_DRAW);
    glVertexAttribPointer(2,         // attribute
                          3,         // size
                          GL_FLOAT,  // type
                          GL_TRUE,   // normalized?
                          0,         // stride
                          (void *) 0 // array buffer offset
    );
    // 3rd attribute buffer : Normals
    glEnableVertexAttribArray(2);
    // Texture Indices
    glGenBuffers(1, &textureIndexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, textureIndexBuffer);
    glBufferData(GL_ARRAY_BUFFER, m_texture_indices.size() * sizeof(unsigned int), &m_texture_indices[0], GL_STATIC_DRAW);
    glVertexAttribIPointer(3, 1, GL_UNSIGNED_INT, 0, (void *) 0);
    // 4th attribute buffer : Texture Indices
    glEnableVertexAttribArray(3);
    // Polygon Flags
    glGenBuffers(1, &polyFlagBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, polyFlagBuffer);
    glBufferData(GL_ARRAY_BUFFER, m_polygon_flags.size() * sizeof(uint32_t), &m_polygon_flags[0], GL_STATIC_DRAW);
    glVertexAttribIPointer(4, 1, GL_UNSIGNED_INT, 0, (void *) 0);
    // 5th attribute buffer : Polygon Flags
    glEnableVertexAttribArray(4);

    glBindVertexArray(0);

    return true;
}
