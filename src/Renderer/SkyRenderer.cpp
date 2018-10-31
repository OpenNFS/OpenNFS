//
// Created by SYSTEM on 09/10/2018.
//

#include "SkyRenderer.h"


SkyRenderer::SkyRenderer(const shared_ptr<ONFSTrack> &activeTrack) {
    // Load track HRZ parameters into shader
    loadTextures();

    // Load OBJ Model
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err;
    std::string warn;
    ASSERT(tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, "../resources/misc/skydome/sphere.obj", nullptr, true, true), err);

    // TODO: Generify the Utils loader to detect norms and uvs, else backfill with vecs of 0's
    for (size_t s = 0; s < shapes.size(); s++) {
        std::vector<glm::vec3> verts = std::vector<glm::vec3>();
        std::vector<glm::vec3> norms = std::vector<glm::vec3>();
        std::vector<glm::vec2> uvs = std::vector<glm::vec2>();
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
                verts.emplace_back(glm::vec3(attrib.vertices[3 * idx.vertex_index + 0] * 400, attrib.vertices[3 * idx.vertex_index + 1] * 400, attrib.vertices[3 * idx.vertex_index + 2] * 400));
                norms.emplace_back(glm::vec3(0.f, 0.f, 0.f)); // Fill the sphere attribs with empty data as missing
                uvs.emplace_back(glm::vec2(0.0f, 0.0f));
            }
            index_offset += fv;
            // per-face material
            // shapes[s].mesh.material_ids[f];
        }
        skydome = CarModel(shapes[s].name + "_obj", verts, uvs, norms, indices, glm::vec3(0, 0, 0), 0.01f, 0.0f, 0.5f);
        break;
    }
    skydome.enable();
    skydome.update();
}

void SkyRenderer::loadTextures() {
    std::string clouds1_texture_path("../resources/misc/skydome/clouds1.tga");
    std::string clouds2_texture_path("../resources/misc/skydome/clouds2.tga");
    std::string sun_texture_path("../resources/misc/skydome/sun.tga");
    std::string moon_texture_path("../resources/misc/skydome/moon.tga");
    std::string tint_texture_path("../resources/misc/skydome/tint.tga");
    std::string tint2_texture_path("../resources/misc/skydome/tint2.tga");
    NS_TGALOADER::IMAGE texture_loader;

    ASSERT(texture_loader.LoadTGA(clouds1_texture_path.c_str()), "Cloud Texture loading failed!");
    glGenTextures(1, &clouds1TextureID);
    glBindTexture(GL_TEXTURE_2D, clouds1TextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_loader.getWidth(), texture_loader.getHeight(), 0, GL_BGRA, GL_UNSIGNED_BYTE, texture_loader.getDataForOpenGL());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    ASSERT(texture_loader.LoadTGA(clouds2_texture_path.c_str()), "Cloud Texture loading failed!");
    glGenTextures(1, &clouds2TextureID);
    glBindTexture(GL_TEXTURE_2D, clouds2TextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_loader.getWidth(), texture_loader.getHeight(), 0, GL_BGRA, GL_UNSIGNED_BYTE, texture_loader.getDataForOpenGL());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    ASSERT(texture_loader.LoadTGA(sun_texture_path.c_str()), "Cloud Texture loading failed!");
    glGenTextures(1, &sunTextureID);
    glBindTexture(GL_TEXTURE_2D, sunTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_loader.getWidth(), texture_loader.getHeight(), 0, GL_BGRA, GL_UNSIGNED_BYTE, texture_loader.getDataForOpenGL());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    ASSERT(texture_loader.LoadTGA(moon_texture_path.c_str()), "Cloud Texture loading failed!");
    glGenTextures(1, &moonTextureID);
    glBindTexture(GL_TEXTURE_2D, moonTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_loader.getWidth(), texture_loader.getHeight(), 0, GL_BGRA, GL_UNSIGNED_BYTE, texture_loader.getDataForOpenGL());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    ASSERT(texture_loader.LoadTGA(tint_texture_path.c_str()), "Cloud Texture loading failed!");
    glGenTextures(1, &tintTextureID);
    glBindTexture(GL_TEXTURE_2D, tintTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_loader.getWidth(), texture_loader.getHeight(), 0, GL_BGRA, GL_UNSIGNED_BYTE, texture_loader.getDataForOpenGL());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    ASSERT(texture_loader.LoadTGA(tint2_texture_path.c_str()), "Cloud Texture loading failed!");
    glGenTextures(1, &tint2TextureID);
    glBindTexture(GL_TEXTURE_2D, tint2TextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_loader.getWidth(), texture_loader.getHeight(), 0, GL_BGRA, GL_UNSIGNED_BYTE, texture_loader.getDataForOpenGL());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);
}

void SkyRenderer::renderSky(const Camera &mainCamera, const Light &sun, const ParamData &userParams, float elapsedTime) {
    skydomeShader.use();
    skydomeShader.loadTextures(clouds1TextureID, clouds2TextureID, sunTextureID, moonTextureID, tintTextureID, tint2TextureID);
    skydomeShader.loadStarRotationMatrix(glm::toMat4(glm::normalize(glm::quat(glm::vec3(SIMD_PI,SIMD_PI,0))))); // No star rotation
    skydomeShader.loadMatrices(mainCamera.ProjectionMatrix, mainCamera.ViewMatrix, skydome.ModelMatrix);
    skydomeShader.loadSunPosition(sun);
    skydomeShader.loadTime(elapsedTime);
    skydomeShader.loadWeatherMixFactor(0.7f);
    // Bind the sphere model
    skydome.render();
    skydomeShader.unbind();
}

SkyRenderer::~SkyRenderer() {
    glDeleteTextures(1, &clouds1TextureID);
    glDeleteTextures(1, &clouds2TextureID);
    glDeleteTextures(1, &sunTextureID);
    glDeleteTextures(1, &moonTextureID);
    glDeleteTextures(1, &tintTextureID);
    glDeleteTextures(1, &tint2TextureID);
    skydomeShader.cleanup();
}


