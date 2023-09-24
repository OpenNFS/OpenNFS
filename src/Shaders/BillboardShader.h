#pragma once

#include "BaseShader.h"
#include <Models/TrackLight.h>

#include "../Util/ImageLoader.h"

class BillboardShader : public BaseShader {
public:
    BillboardShader();
    void loadLight(const std::shared_ptr<TrackLight> &light);
    void loadMatrices(const glm::mat4 &projection, const glm::mat4 &view);

protected:
    void bindAttributes() override;
    void getAllUniformLocations() override;
    void customCleanup() override;

    GLint projectionMatrixLocation;
    GLint viewMatrixLocation;
    GLint boardTextureLocation;
    GLint lightColourLocation;
    GLint billboardPosLocation;

    GLuint textureID;

    typedef BaseShader super;

    void loadBillboardTexture();

    void load_bmp_texture();
};
