#pragma once

#include "GLModel.h"
#include <Entities/TrackGeometry.h>

class GLTrackModel : public GLModel, public LibOpenNFS::TrackGeometry {
  public:
    explicit GLTrackModel(TrackGeometry const &geometry) : TrackGeometry(geometry) {
    }
    GLTrackModel() = default;
    ~GLTrackModel() override;

    // Non-copyable (OpenGL resources)
    GLTrackModel(GLTrackModel const &) = delete;
    GLTrackModel &operator=(GLTrackModel const &) = delete;
    // Movable
    GLTrackModel(GLTrackModel &&other) noexcept;
    GLTrackModel &operator=(GLTrackModel &&other) noexcept;

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
