#include "TrackLoader.h"

#include "NFS3/NFS3Loader.h"
#include "NFS2/NFS2Loader.h"
/*#include "NFS4/PC/NFS4Loader.h"
#include "NFS4/PS1/NFS4PS1Loader.h"*/

std::shared_ptr<Track> TrackLoader::LoadTrack(NFSVer trackVersion, const std::string &trackName)
{
    std::shared_ptr<Track> loadedTrack;

    std::stringstream trackPath;
    trackPath << RESOURCE_PATH << ToString(trackVersion);

    switch (trackVersion)
    {
    case NFS_2:
        trackPath << NFS_2_TRACK_PATH << trackName;
        loadedTrack = NFS2Loader<LibOpenNFS::NFS2::PC>::LoadTrack(trackPath.str());
        break;
    case NFS_2_SE:
        trackPath << NFS_2_SE_TRACK_PATH << trackName;
        loadedTrack = NFS2Loader<LibOpenNFS::NFS2::PC>::LoadTrack(trackPath.str());
        break;
    case NFS_3:
        trackPath << NFS_3_TRACK_PATH << trackName;
        loadedTrack = NFS3Loader::LoadTrack(trackPath.str());
        break;
    case NFS_3_PS1:
        trackPath << "/" << trackName;
        loadedTrack = NFS2Loader<LibOpenNFS::NFS2::PS1>::LoadTrack(trackPath.str());
        break;
        /* case NFS_4:
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
             break;*/
    case UNKNOWN:
        ASSERT(false, "Unknown track type!");
    default:
        break;
    }

    loadedTrack->GenerateSpline();
    loadedTrack->GenerateAabbTree();

    return loadedTrack;
}