#include "SkidMarkRenderer.h"
#include "../Race/SkidMarkSystem.h"

#include <cstring>

namespace OpenNFS {
    SkidMarkRenderer::SkidMarkRenderer() {
        InitBuffers();
        GenerateDefaultTexture();
    }

    SkidMarkRenderer::~SkidMarkRenderer() {
        if (m_vao != 0) {
            glDeleteVertexArrays(1, &m_vao);
        }
        if (m_vbo != 0) {
            glDeleteBuffers(1, &m_vbo);
        }
        if (m_defaultTexture != 0) {
            glDeleteTextures(1, &m_defaultTexture);
        }
        m_shader.cleanup();
    }

    void SkidMarkRenderer::InitBuffers() {
        glGenVertexArrays(1, &m_vao);
        glGenBuffers(1, &m_vbo);

        glBindVertexArray(m_vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

        // Allocate buffer with GL_DYNAMIC_DRAW for frequent updates
        glBufferData(GL_ARRAY_BUFFER, MAX_VERTICES * sizeof(SkidMarkVertex), nullptr, GL_DYNAMIC_DRAW);

        // Vertex attributes
        // Position (location 0)
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SkidMarkVertex),
                              reinterpret_cast<void *>(offsetof(SkidMarkVertex, position)));

        // UV (location 1)
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(SkidMarkVertex),
                              reinterpret_cast<void *>(offsetof(SkidMarkVertex, uv)));

        // Normal (location 2)
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(SkidMarkVertex),
                              reinterpret_cast<void *>(offsetof(SkidMarkVertex, normal)));

        // Age (location 3)
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(SkidMarkVertex),
                              reinterpret_cast<void *>(offsetof(SkidMarkVertex, age)));

        // Intensity (location 4)
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(SkidMarkVertex),
                              reinterpret_cast<void *>(offsetof(SkidMarkVertex, intensity)));

        // Surface type (location 5)
        glEnableVertexAttribArray(5);
        glVertexAttribIPointer(5, 1, GL_UNSIGNED_INT, sizeof(SkidMarkVertex),
                               reinterpret_cast<void *>(offsetof(SkidMarkVertex, surfaceType)));

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void SkidMarkRenderer::GenerateDefaultTexture() {
        // Create a simple procedural skid mark texture
        constexpr int texSize = 64;
        std::vector<uint8_t> texData(texSize * texSize * 4);

        for (int y = 0; y < texSize; ++y) {
            for (int x = 0; x < texSize; ++x) {
                int const idx = (y * texSize + x) * 4;

                // Create a gradient that's darker in the center (tire contact area)
                float const fx = static_cast<float>(x) / texSize;
                float const fy = static_cast<float>(y) / texSize;

                // Distance from center line (x = 0.5)
                float distFromCenter = std::abs(fx - 0.5f) * 2.0f;

                // Softer edges with a smooth falloff
                float alpha = 1.0f - std::pow(distFromCenter, 2.0f);
                alpha = std::max(0.0f, alpha);

                // Add some noise/variation
                float noise = (std::sin(fx * 20.0f) * std::cos(fy * 20.0f) + 1.0f) * 0.1f;
                alpha = std::min(1.0f, alpha + noise * 0.2f);

                // RGB is white (tint applied in shader)
                texData[idx + 0] = 255;
                texData[idx + 1] = 255;
                texData[idx + 2] = 255;
                texData[idx + 3] = static_cast<uint8_t>(alpha * 255);
            }
        }

        glGenTextures(1, &m_defaultTexture);
        glBindTexture(GL_TEXTURE_2D, m_defaultTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texSize, texSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void SkidMarkRenderer::BuildVertexData(std::vector<VehicleTrails> const &allTrails) {
        m_vertices.clear();
        m_vertices.reserve(MAX_VERTICES);

        for (auto const &[wheelTrails, vehicleID] : allTrails) {
            for (auto const &wheelTrail : wheelTrails) {
                if (wheelTrail.count < 2) {
                    continue; // Need at least 2 segments for a strip
                }

                bool firstSegmentInStrip = true;

                // Iterate through the ring buffer
                for (size_t i = 0; i < wheelTrail.count - 1 && m_vertices.size() < MAX_VERTICES - 4; ++i) {
                    size_t currentIdx = (wheelTrail.headIndex + i) % wheelTrail.segments.size();
                    size_t nextIdx = (wheelTrail.headIndex + i + 1) % wheelTrail.segments.size();

                    auto const &current = wheelTrail.segments[currentIdx];
                    auto const &next = wheelTrail.segments[nextIdx];

                    if (!current.valid || !next.valid) {
                        firstSegmentInStrip = true;
                        continue;
                    }

                    // If starting a new strip after a gap, add degenerate triangles
                    if (firstSegmentInStrip && !m_vertices.empty()) {
                        // Degenerate: repeat last vertex
                        m_vertices.push_back(m_vertices.back());

                        // Degenerate: add first vertex of new strip
                        SkidMarkVertex degenerateStart{};
                        degenerateStart.position = current.leftPosition;
                        degenerateStart.uv = glm::vec2(0.0f, 0.0f);
                        degenerateStart.normal = current.normal;
                        degenerateStart.age = current.age;
                        degenerateStart.intensity = current.intensity;
                        degenerateStart.surfaceType = static_cast<uint32_t>(current.surface);
                        m_vertices.push_back(degenerateStart);
                    }

                    // Calculate UV coordinates
                    float vCoord = static_cast<float>(i) / static_cast<float>(wheelTrail.count - 1);

                    // Add left and right vertices of current segment
                    SkidMarkVertex leftCurrent{};
                    leftCurrent.position = current.leftPosition;
                    leftCurrent.uv = glm::vec2(0.0f, vCoord);
                    leftCurrent.normal = current.normal;
                    leftCurrent.age = current.age;
                    leftCurrent.intensity = current.intensity;
                    leftCurrent.surfaceType = static_cast<uint32_t>(current.surface);
                    m_vertices.push_back(leftCurrent);

                    SkidMarkVertex rightCurrent{};
                    rightCurrent.position = current.rightPosition;
                    rightCurrent.uv = glm::vec2(1.0f, vCoord);
                    rightCurrent.normal = current.normal;
                    rightCurrent.age = current.age;
                    rightCurrent.intensity = current.intensity;
                    rightCurrent.surfaceType = static_cast<uint32_t>(current.surface);
                    m_vertices.push_back(rightCurrent);

                    firstSegmentInStrip = false;
                }
            }
        }
    }

    void SkidMarkRenderer::Render(BaseCamera const &camera, std::vector<VehicleTrails> const &allTrails,
                                   float const lifetime, float const fadeStart) {
        if (allTrails.empty()) {
            return;
        }

        // Build vertex data from trails
        BuildVertexData(allTrails);

        if (m_vertices.empty()) {
            return;
        }

        // Upload vertex data
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, MAX_VERTICES * sizeof(SkidMarkVertex), nullptr, GL_DYNAMIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, m_vertices.size() * sizeof(SkidMarkVertex), m_vertices.data());
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Set up render state for decal-like rendering
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(-1.0f, -1.0f);  // Slight offset to prevent z-fighting with track

        // Use shader and set uniforms
        m_shader.use();
        m_shader.loadProjectionViewMatrices(camera.projectionMatrix, camera.viewMatrix);
        m_shader.loadLifetime(lifetime);
        m_shader.loadFadeStart(fadeStart);
        m_shader.bindSkidTexture(m_defaultTexture);

        // Draw the triangle strip
        glBindVertexArray(m_vao);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, static_cast<GLsizei>(m_vertices.size()));
        glBindVertexArray(0);

        m_shader.unbind();
        m_shader.HotReload();

        // Restore render state
        glDisable(GL_POLYGON_OFFSET_FILL);
        glDepthMask(GL_TRUE);
    }
} // namespace OpenNFS
