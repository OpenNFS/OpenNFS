#include "SkyRenderer.h"

#include <Util/ModelLoader.h>
#include <glm/gtx/quaternion.hpp>

namespace OpenNFS {
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
        clouds1TextureID = ImageLoader::LoadImage(clouds1_texture_path, &width, &height, GL_CLAMP_TO_BORDER,
                                                  GL_LINEAR_MIPMAP_LINEAR);
        clouds2TextureID = ImageLoader::LoadImage(clouds2_texture_path, &width, &height, GL_CLAMP_TO_BORDER,
                                                  GL_LINEAR_MIPMAP_LINEAR);
        sunTextureID = ImageLoader::LoadImage(sun_texture_path, &width, &height, GL_CLAMP_TO_BORDER,
                                              GL_LINEAR_MIPMAP_LINEAR);
        moonTextureID = ImageLoader::LoadImage(moon_texture_path, &width, &height, GL_CLAMP_TO_BORDER,
                                               GL_LINEAR_MIPMAP_LINEAR);
        tintTextureID = ImageLoader::LoadImage(tint_texture_path, &width, &height, GL_CLAMP_TO_BORDER,
                                               GL_LINEAR_MIPMAP_LINEAR);
        tint2TextureID = ImageLoader::LoadImage(tint2_texture_path, &width, &height, GL_CLAMP_TO_BORDER,
                                                GL_LINEAR_MIPMAP_LINEAR);

        // Load OBJ Model
        std::vector<ModelData> models;
        CHECK_F(ModelLoader::LoadObj("../resources/misc/skydome/sphere.obj", models, 400.0f), "Failed to load skydome model");

        for (const auto& model : models) {
             m_skydomeGeom = CarGeometry("skydome_obj", model.m_vertices, model.m_uvs, model.m_normals, model.m_indices, glm::vec3(0, 0, 0));
             m_skydomeModel = GLCarModel(m_skydomeGeom, 0.01f, 0.0f, 0.5f);
             break;
        }
        m_skydomeModel.Enable();
        m_skydomeModel.UpdateMatrices();
    }

    void SkyRenderer::Render(const BaseCamera &camera, const GlobalLight* light,
                             const float elapsedTime) {
        m_skydomeShader.use();
        m_skydomeShader.loadTextures(clouds1TextureID, clouds2TextureID, sunTextureID, moonTextureID, tintTextureID,
                                     tint2TextureID);
        m_skydomeShader.loadStarRotationMatrix(
            glm::toMat3(glm::normalize(glm::quat(glm::vec3(glm::pi<float>(), glm::pi<float>(), 0)))));
        // No star rotation
        m_skydomeShader.loadMatrices(camera.projectionMatrix, camera.viewMatrix, m_skydomeModel.ModelMatrix);
        m_skydomeShader.loadSunPosition(light);
        m_skydomeShader.loadTime(elapsedTime);
        m_skydomeShader.loadWeatherMixFactor(1.0f);
        // Bind the sphere model
        m_skydomeModel.Render();
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
} // namespace OpenNFS
