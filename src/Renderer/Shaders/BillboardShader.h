#pragma once

#include "BaseShader.h"
#include <Entities/TrackLight.h>

#include "../../Util/ImageLoader.h"

namespace OpenNFS {
    class BillboardShader : public BaseShader {
      public:
        BillboardShader();
        void loadLight(LibOpenNFS::TrackLight const *light);
        void loadMatrices(glm::mat4 const &projection, glm::mat4 const &view);

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

        void loadBillboardTexture();

        void load_bmp_texture();
    };
} // namespace OpenNFS
