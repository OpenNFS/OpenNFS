#pragma once

#include "../Util/ImageLoader.h"
#include "../Loaders/TrackLoader.h"
#include "../Shaders/SkydomeShader.h"
#include "../Camera/Camera.h"
#include "../Scene/GlobalLight.h"

class SkyRenderer {
public:
    explicit SkyRenderer();
    ~SkyRenderer();
    void Render(const std::shared_ptr<Camera> &camera, const GlobalLight &sun, float elapsedTime);
private:
    // Load cloud, sun, moon and tint textures
    void _LoadAssets();
    SkydomeShader m_skydomeShader;
    // Sphere model for skydome
    CarModel m_skydomeModel;
    GLuint clouds1TextureID = 0, clouds2TextureID = 0, sunTextureID = 0, moonTextureID = 0, tintTextureID = 0, tint2TextureID = 0;
};