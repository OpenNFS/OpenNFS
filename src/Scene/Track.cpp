#include "Track.h"

#include "../Config.h"
#include "Common/TextureUtils.h"

namespace OpenNFS {
    Track::Track(const LibOpenNFS::Track &track) : LibOpenNFS::Track(track), cullTree(kCullTreeInitialSize) {
        assetPath = TRACK_PATH + get_string(nfsVersion) + "/" + name;
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
            textureMap[trackTexture.id] = GLTexture::LoadTexture(nfsVersion, trackTexture);
        }
        textureArrayID = GLTexture::MakeTextureArray(textureMap, false);
    }

    void Track::_GenerateSpline() {
        // Build a spline through the center of the track
        std::vector<glm::vec3> cameraPoints;
        for (auto &trackBlock : trackBlocks) {
            cameraPoints.emplace_back(trackBlock.position.x, trackBlock.position.y + 0.2, trackBlock.position.z);
        }
        centerSpline = HermiteCurve(cameraPoints, 0.1f, 0.0f);
    }

    void Track::_GenerateAabbTree() {
        // Build an optimised BvH of AABB's so that culling for render becomes cheap
        for (auto &trackBlock : trackBlocks) {
            for (auto &baseTrackEntity : trackBlock.track) {
                cullTree.insertObject(std::make_shared<Entity>(baseTrackEntity));
            }
            for (auto &trackObjectEntity : trackBlock.objects) {
                cullTree.insertObject(std::make_shared<Entity>(trackObjectEntity));
            }
            for (auto &trackLaneEntity : trackBlock.lanes) {
                cullTree.insertObject(std::make_shared<Entity>(trackLaneEntity));
            }
        }
    }
} // namespace OpenNFS
