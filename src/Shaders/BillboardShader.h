//
// Created by Amrik Sadhra on 11/03/2018.
//

#ifndef OPENNFS3_BILLBOARDSHADER_H
#define OPENNFS3_BILLBOARDSHADER_H


#include "BaseShader.h"
#include "../Util/Utils.h"
#include "../Scene/Light.h"

class BillboardShader : public BaseShader {
public:
    BillboardShader();
    void loadLight(Light board_light);
    void loadMatrices(const glm::mat4 &projection, const glm::mat4 &view, const glm::mat4 &transformation);

protected:
    void bindAttributes() override;
    void getAllUniformLocations() override;
    void customCleanup() override;
    GLint transformationMatrixLocation;
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


#endif //OPENNFS3_BILLBOARDSHADER_H
