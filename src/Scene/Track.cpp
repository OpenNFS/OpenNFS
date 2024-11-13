#include "Track.h"

namespace OpenNFS {
    Track::Track(const LibOpenNFS::Track &track) : LibOpenNFS::Track(track), cullTree(kCullTreeInitialSize) {
        this->_LoadTextures();
        this->_GenerateSpline();
        this->_GenerateAabbTree();
    }

    Track::~Track() {
        glDeleteTextures(textureMap.size(), &textureArrayID);
    }

    void Track::_LoadTextures() {
        switch (nfsVersion) {
        case NFSVersion::UNKNOWN:
            break;
        case NFSVersion::NFS_1:
            break;
        case NFSVersion::NFS_2:
            break;
        case NFSVersion::NFS_2_PS1:
            break;
        case NFSVersion::NFS_2_SE:
            break;
        case NFSVersion::NFS_3:
            CHECK_F(GLTexture::ExtractTrackTextures(trackBasePath, trackNameStripped, NFSVersion::NFS_3), "Could not extract " << trackNameStripped << " QFS texture pack");
            // Load QFS textures into GL objects
            for (auto &frdTexBlock : frdFile.textureBlocks) {
                textureMap[frdTexBlock.qfsIndex] = GLTexture::LoadTexture(NFSVersion::NFS_3, frdTexBlock, trackNameStripped);
            }
            textureArrayID = GLTexture::MakeTextureArray(textureMap, false);
            break;
        case NFSVersion::NFS_3_PS1:
            break;
        case NFSVersion::NFS_4:
            break;
        case NFSVersion::NFS_4_PS1:
            break;
        case NFSVersion::MCO:
            break;
        case NFSVersion::NFS_5:
            break;
        }
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
