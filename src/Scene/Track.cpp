#include "Track.h"

#include "Common/TextureUtils.h"

namespace OpenNFS {
    Track::Track(const LibOpenNFS::Track &track) : LibOpenNFS::Track(track), cullTree(kCullTreeInitialSize) {
        assetPath = TRACK_PATH + get_string(nfsVersion) + "/" + name;
        this->_GenerateEntities();
        this->_LoadTextures();
        this->_GenerateSpline();
        this->_GenerateAabbTree();
    }

    Track::~Track() {
        glDeleteTextures((GLsizei) textureMap.size(), &textureArrayID);
    }

    void Track::_LoadTextures() {
        CHECK_F(LibOpenNFS::TextureUtils::ExtractTrackTextures(basePath, name, nfsVersion, assetPath), "Could not extract %s texture pack", name.c_str());
        // Load textures into GL objects
        for (auto &trackTexture : trackTextures) {
            textureMap[trackTexture.first] = GLTexture::LoadTexture(nfsVersion, trackTexture.second);
        }
        textureArrayID = GLTexture::MakeTextureArray(textureMap, false);
    }

    void Track::_GenerateEntities() {
        for (auto &trackBlock : trackBlocks) {
            for (auto &trackObject : trackBlock.objects) {
                entities.emplace_back(std::make_shared<Entity>(&trackObject));
            }
            for (auto &trackSurface : trackBlock.track) {
                entities.emplace_back(std::make_shared<Entity>(&trackSurface));
            }
            for (auto &trackLane : trackBlock.lanes) {
                entities.emplace_back(std::make_shared<Entity>(&trackLane));
            }
        }
        // Update all models at least once to ensure GL Buffers are generated
        for (auto &entity : entities) {
            entity->model->update();
        }
    }

    void Track::_GenerateSpline() {
        // Build a spline through the center of the track
        std::vector<glm::vec3> cameraPoints;
        cameraPoints.reserve(trackBlocks.size());
        for (auto &trackBlock : trackBlocks) {
            cameraPoints.emplace_back(trackBlock.position.x, trackBlock.position.y + 0.2, trackBlock.position.z);
        }
        centerSpline = HermiteCurve(cameraPoints, 0.1f, 0.0f);
    }

    void Track::_GenerateAabbTree() {
        // Build an optimised BvH of AABB's so that culling for render becomes cheap
        for (auto &entity : entities) {
            // cullTree.insertObject(entity);
        }
    }
} // namespace OpenNFS
