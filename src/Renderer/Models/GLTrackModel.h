#pragma once

#include "GLModel.h"
#include <Entities/TrackGeometry.h>

class GLTrackModel : public GLModel, public LibOpenNFS::TrackGeometry {
public:
    GLTrackModel(const TrackGeometry &geometry) : TrackGeometry(geometry) {
    }
    GLTrackModel() {
    }
    ~GLTrackModel() override;
    bool GenBuffers() override;
    void UpdateMatrices() override;
    void Render() override;

private:
    GLuint m_vertexBuffer{};
    GLuint m_uvBuffer{};
    GLuint m_textureIndexBuffer{};
    GLuint m_shadingDataBuffer{};
    GLuint m_normalBuffer{};
    GLuint m_debugBuffer{};
};
