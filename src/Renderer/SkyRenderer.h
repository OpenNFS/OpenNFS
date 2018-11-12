//
// Created by Amrik Sadhra on 09/10/2018.
//

#pragma once

#include "../../include/TGALoader/TGALoader.h"
#include "../Loaders/trk_loader.h"
#include "../Scene/Camera.h"
#include "../Shaders/SkydomeShader.h"

class SkyRenderer {
  public:
    explicit SkyRenderer(const shared_ptr<ONFSTrack> &activeTrack);
    ~SkyRenderer();
    void renderSky(const Camera &mainCamera, const Light &sun, const ParamData &userParams, float elapsedTime);

  private:
    // Create and compile our GLSL programs from the shaders
    SkydomeShader skydomeShader;
    shared_ptr<ONFSTrack> track;

    // Sphere model for skydome
    CarModel skydome;

    GLuint clouds1TextureID = 0, clouds2TextureID = 0, sunTextureID = 0, moonTextureID = 0, tintTextureID = 0,
           tint2TextureID = 0;

    // Load cloud, sun, moon and tint textures
    void loadTextures();
};