#include "SkyRenderer.h"

SkyRenderer::SkyRenderer() {
    // Load track HRZ parameters into shader
    this->_LoadAssets();
}

void SkyRenderer::_LoadAssets() {
    std::string clouds1_texture_path("../resources/misc/skydome/clouds1.tga");
    std::string clouds2_texture_path("../resources/misc/skydome/clouds2.tga");
    std::string sun_texture_path("../resources/misc/skydome/sun.tga");
    std::string moon_texture_path("../resources/misc/skydome/moon.tga");
    std::string tint_texture_path("../resources/misc/skydome/tint.tga");
    std::string tint2_texture_path("../resources/misc/skydome/tint2.tga");
    int width, height;
    clouds1TextureID = ImageLoader::LoadImage(clouds1_texture_path, &width, &height, GL_CLAMP_TO_BORDER, GL_LINEAR_MIPMAP_LINEAR);
    clouds2TextureID = ImageLoader::LoadImage(clouds2_texture_path, &width, &height, GL_CLAMP_TO_BORDER, GL_LINEAR_MIPMAP_LINEAR);
    sunTextureID     = ImageLoader::LoadImage(sun_texture_path, &width, &height, GL_CLAMP_TO_BORDER, GL_LINEAR_MIPMAP_LINEAR);
    moonTextureID    = ImageLoader::LoadImage(moon_texture_path, &width, &height, GL_CLAMP_TO_BORDER, GL_LINEAR_MIPMAP_LINEAR);
    tintTextureID    = ImageLoader::LoadImage(tint_texture_path, &width, &height, GL_CLAMP_TO_BORDER, GL_LINEAR_MIPMAP_LINEAR);
    tint2TextureID   = ImageLoader::LoadImage(tint2_texture_path, &width, &height, GL_CLAMP_TO_BORDER, GL_LINEAR_MIPMAP_LINEAR);

    // Load OBJ Model
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err;
    std::string warn;
    ASSERT(tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, "../resources/misc/skydome/sphere.obj", nullptr, true, true), err);

    // TODO: Generify the Utils loader to detect norms and uvs, else backfill with vecs of 0's
    for (size_t s = 0; s < shapes.size(); s++) {
        std::vector<glm::vec3> verts      = std::vector<glm::vec3>();
        std::vector<glm::vec3> norms      = std::vector<glm::vec3>();
        std::vector<glm::vec2> uvs        = std::vector<glm::vec2>();
        std::vector<unsigned int> indices = std::vector<unsigned int>();
        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            int fv = shapes[s].mesh.num_face_vertices[f];
            // Loop over vertices in the face.
            for (size_t v = 0; v < fv; v++) {
                // access to vertex
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                indices.emplace_back((const unsigned int &) idx.vertex_index);
                verts.emplace_back(
                  glm::vec3(attrib.vertices[3 * idx.vertex_index + 0] * 400, attrib.vertices[3 * idx.vertex_index + 1] * 400, attrib.vertices[3 * idx.vertex_index + 2] * 400));
                norms.emplace_back(glm::vec3(0.f, 0.f, 0.f)); // Fill the sphere attribs with empty data as missing
                uvs.emplace_back(glm::vec2(0.0f, 0.0f));
            }
            index_offset += fv;
            // per-face material
            shapes[s].mesh.material_ids[f];
        }
        m_skydomeModel = CarModel(shapes[s].name + "_obj", verts, uvs, norms, indices, glm::vec3(0, 0, 0), 0.01f, 0.0f, 0.5f);
        break;
    }
    m_skydomeModel.enable();
    m_skydomeModel.update();
}

void SkyRenderer::Render(const BaseCamera &camera, const std::shared_ptr<GlobalLight> &light, float elapsedTime) {
    m_skydomeShader.use();
    m_skydomeShader.loadTextures(clouds1TextureID, clouds2TextureID, sunTextureID, moonTextureID, tintTextureID, tint2TextureID);
    m_skydomeShader.loadStarRotationMatrix(glm::toMat3(glm::normalize(glm::quat(glm::vec3(SIMD_PI, SIMD_PI, 0))))); // No star rotation
    m_skydomeShader.loadMatrices(camera.projectionMatrix, camera.viewMatrix, m_skydomeModel.ModelMatrix);
    m_skydomeShader.loadSunPosition(light);
    m_skydomeShader.loadTime(elapsedTime);
    m_skydomeShader.loadWeatherMixFactor(1.0f);
    // Bind the sphere model
    m_skydomeModel.render();
    m_skydomeShader.unbind();
    m_skydomeShader.HotReload();
}

SkyRenderer::~SkyRenderer() {
    glDeleteTextures(1, &clouds1TextureID);
    glDeleteTextures(1, &clouds2TextureID);
    glDeleteTextures(1, &sunTextureID);
    glDeleteTextures(1, &moonTextureID);
    glDeleteTextures(1, &tintTextureID);
    glDeleteTextures(1, &tint2TextureID);
    m_skydomeShader.cleanup();
}
