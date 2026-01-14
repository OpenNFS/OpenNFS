#include "Track.h"

#include "Common/TextureUtils.h"

namespace OpenNFS {
    Track::Track(LibOpenNFS::Track const &track) : LibOpenNFS::Track(track), cullTree(kCullTreeInitialSize) {
        assetPath = TRACK_PATH + magic_enum::enum_name(nfsVersion).data() + "/" + name;
        this->_GenerateEntities();
        this->_LoadTextures();
        this->_GenerateSpline();
        this->_GenerateAabbTree();
    }

    Track::~Track() {
        glDeleteTextures(1, &textureArrayID);
    }

    void Track::_LoadTextures() {
        CHECK_F(LibOpenNFS::TextureUtils::ExtractTrackTextures(basePath, name, nfsVersion, assetPath), "Could not extract %s texture pack",
                name.c_str());
        // Load textures into GL objects
        for (auto &[id, trackTextureAsset] : trackTextureAssets) {
            textureMap[id] = GLTexture::LoadTexture(nfsVersion, trackTextureAsset);
        }
        textureArrayID = GLTexture::MakeTextureArray(textureMap, false);
    }

    void Track::_GenerateEntities() {
        perTrackblockEntities.resize(trackBlocks.size());
        for (auto &trackBlock : trackBlocks) {
            for (auto &trackObject : trackBlock.objects) {
                perTrackblockEntities.at(trackBlock.id).emplace_back(std::make_shared<Entity>(trackObject));
            }
            for (auto &trackSurface : trackBlock.track) {
                perTrackblockEntities.at(trackBlock.id).emplace_back(std::make_shared<Entity>(trackSurface));
            }
            for (auto &trackLane : trackBlock.lanes) {
                perTrackblockEntities.at(trackBlock.id).emplace_back(std::make_shared<Entity>(trackLane));
            }
            for (auto &trackLight : trackBlock.lights) {
                perTrackblockEntities.at(trackBlock.id).emplace_back(std::make_shared<Entity>(trackLight));
            }
            for (auto &trackGlobalObject : globalObjects) {
                perTrackblockEntities.at(trackBlock.id).emplace_back(std::make_shared<Entity>(trackGlobalObject));
            }
        }
        // Enable all models to ensure GL Buffers are generated for track
        for (auto const &trackBlockEntities : perTrackblockEntities) {
            for (auto const &entity : trackBlockEntities) {
                entity->Enable();
                entity->UpdateMatrices();
            }
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
        for (auto const &trackBlockEntities : perTrackblockEntities) {
            for (auto const &entity : trackBlockEntities) {
                cullTree.insertObject(entity);
            }
        }
    }
} // namespace OpenNFS
