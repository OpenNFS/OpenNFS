#include "TrackLoader.h"
#include "../Physics/AABBTree.h"

#include <memory>

ONFSTrack::ONFSTrack(NFSVer trackVersion, const std::string &trackName)
{
    tag = trackVersion;
    name = trackName;

    std::stringstream trackPath;
    trackPath << RESOURCE_PATH << ToString(tag);

    switch (tag)
    {
        case NFS_2:
            trackPath << NFS_2_TRACK_PATH << trackName;
            trackData = NFS2<PC>::LoadTrack(trackPath.str());
            nBlocks = boost::get<std::shared_ptr<NFS2_DATA::PC::TRACK>>(trackData)->nBlocks;
            cameraAnimations = boost::get<std::shared_ptr<NFS2_DATA::PC::TRACK>>(trackData)->cameraAnimation;
            textureArrayID = boost::get<std::shared_ptr<NFS2_DATA::PC::TRACK>>(trackData)->textureArrayID;
            trackBlocks = boost::get<std::shared_ptr<NFS2_DATA::PC::TRACK>>(trackData)->track_blocks;
            globalObjects = boost::get<std::shared_ptr<NFS2_DATA::PC::TRACK>>(trackData)->global_objects;
            break;
        case NFS_2_SE:
            trackPath << NFS_2_SE_TRACK_PATH << trackName;
            trackData = NFS2<PC>::LoadTrack(trackPath.str());
            nBlocks = boost::get<std::shared_ptr<NFS2_DATA::PC::TRACK>>(trackData)->nBlocks;
            cameraAnimations = boost::get<std::shared_ptr<NFS2_DATA::PC::TRACK>>(trackData)->cameraAnimation;
            textureArrayID = boost::get<std::shared_ptr<NFS2_DATA::PC::TRACK>>(trackData)->textureArrayID;
            trackBlocks = boost::get<std::shared_ptr<NFS2_DATA::PC::TRACK>>(trackData)->track_blocks;
            globalObjects = boost::get<std::shared_ptr<NFS2_DATA::PC::TRACK>>(trackData)->global_objects;
            break;
        case NFS_3:
            trackPath << NFS_3_TRACK_PATH << trackName;
            trackData = NFS3::LoadTrack(trackPath.str());
            nBlocks = boost::get<std::shared_ptr<NFS3_4_DATA::TRACK>>(trackData)->nBlocks;
            cameraAnimations = boost::get<std::shared_ptr<NFS3_4_DATA::TRACK>>(trackData)->cameraAnimation;
            textureArrayID =  boost::get<std::shared_ptr<NFS3_4_DATA::TRACK>>(trackData)->textureArrayID;
            trackBlocks = boost::get<std::shared_ptr<NFS3_4_DATA::TRACK>>(trackData)->track_blocks;
            globalObjects = boost::get<std::shared_ptr<NFS3_4_DATA::TRACK>>(trackData)->global_objects;
            break;
        case NFS_3_PS1:
            trackPath << "/" << trackName;
            trackData = NFS2<NFS2_DATA::PS1>::LoadTrack(trackPath.str());
            nBlocks = boost::get<std::shared_ptr<NFS2_DATA::PS1::TRACK>>(trackData)->nBlocks;
            cameraAnimations = boost::get<std::shared_ptr<NFS2_DATA::PS1::TRACK>>(trackData)->cameraAnimation;
            textureArrayID =  boost::get<std::shared_ptr<NFS2_DATA::PS1::TRACK>>(trackData)->textureArrayID;
            trackBlocks = boost::get<std::shared_ptr<NFS2_DATA::PS1::TRACK>>(trackData)->track_blocks;
            globalObjects = boost::get<std::shared_ptr<NFS2_DATA::PS1::TRACK>>(trackData)->global_objects;
            break;
        case NFS_4:
            trackPath << NFS_4_TRACK_PATH << trackName;
            trackData = NFS4::LoadTrack(trackPath.str());
            nBlocks = boost::get<std::shared_ptr<NFS3_4_DATA::TRACK>>(trackData)->nBlocks;
            cameraAnimations = boost::get<std::shared_ptr<NFS3_4_DATA::TRACK>>(trackData)->cameraAnimation;
            textureArrayID = boost::get<std::shared_ptr<NFS3_4_DATA::TRACK>>(trackData)->textureArrayID;
            trackBlocks = boost::get<std::shared_ptr<NFS3_4_DATA::TRACK>>(trackData)->track_blocks;
            globalObjects = boost::get<std::shared_ptr<NFS3_4_DATA::TRACK>>(trackData)->global_objects;
            break;
        case NFS_4_PS1:
            trackPath << "/" << trackName << ".GRP";
            trackData = NFS4PS1::LoadTrack(trackPath.str());
            ASSERT(false, "Implement!");
            break;
        case UNKNOWN:
            ASSERT(false, "Unknown track type!");
        default:
            break;
    }

    this->_GenerateSpline();
    this->_GenerateAabbTree();
}

void ONFSTrack::_GenerateSpline()
{
    // Build a spline through the center of the track
    std::vector<glm::vec3> cameraPoints;
    for (auto &trackBlock : trackBlocks)
    {
        cameraPoints.emplace_back(glm::vec3(trackBlock.center.x, trackBlock.center.y + 0.2, trackBlock.center.z));
    }
    centerSpline = HermiteCurve(cameraPoints, 0.1f, 0.0f);
}

void ONFSTrack::_GenerateAabbTree()
{
    AABBTree cullTree(trackBlocks.size() * 30);
    // Render the per-trackblock data
    for (auto &trackBlock : trackBlocks)
    {
        for (auto &baseTrackEntity : trackBlock.track)
        {
            //cullTree.insertObject(baseTrackEntity);
        }
        for (auto &trackObjectEntity : trackBlock.objects)
        {
            //cullTree.insertObject(trackObjectEntity);
        }
        for (auto &trackLaneEntity : trackBlock.lanes)
        {
            //cullTree.insertObject(trackLaneEntity);
        }
    }
}

std::shared_ptr<ONFSTrack> TrackLoader::LoadTrack(NFSVer nfs_version, const std::string &track_name)
{
    return std::make_shared<ONFSTrack>(nfs_version, track_name);
}

