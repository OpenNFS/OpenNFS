#pragma once

#include "../Util/ImageLoader.h"
#include "../Loaders/TrackLoader.h"
#include "../Shaders/SkydomeShader.h"
#include "../Scene/Camera.h"
#include "../Scene/GlobalLight.h"

class SkyRenderer {
public:
    explicit SkyRenderer();
    ~SkyRenderer();
    void Render(const Camera &mainCamera, const GlobalLight &sun, float elapsedTime);
private:
    // Load cloud, sun, moon and tint textures
    void LoadTextures();
    SkydomeShader skydomeShader;
    // Sphere model for skydome
    CarModel skydome;
    GLuint clouds1TextureID = 0, clouds2TextureID = 0, sunTextureID = 0, moonTextureID = 0, tintTextureID = 0, tint2TextureID = 0;
};