#pragma once

#include "GLModel.h"
#include <Entities/TrackGeometry.h>

class GLTrackModel : public GLModel {
public:
    GLTrackModel(LibOpenNFS::TrackGeometry *geometry) : GLModel(geometry) {
    }
    GLTrackModel() : GLModel() {
    }
    ~GLTrackModel() {
        destroy();
    };

    void update() override;
    void destroy() override;
    void render() override;
    bool genBuffers() override;

private:
    GLuint m_vertexBuffer;
    GLuint m_uvBuffer;
    GLuint m_textureIndexBuffer;
    GLuint m_shadingDataBuffer;
    GLuint m_normalBuffer;
    GLuint m_debugBuffer;
};
