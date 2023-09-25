#include "TrackLoader.h"

#include "NFS3/NFS3Loader.h"
#include "NFS2/NFS2Loader.h"
/*#include "NFS4/PC/NFS4Loader.h"
#include "NFS4/PS1/NFS4PS1Loader.h"*/

namespace OpenNFS {
    std::shared_ptr<Track> TrackLoader::LoadTrack(NFSVersion trackVersion, const std::string &trackName) {
        std::shared_ptr<Track> loadedTrack;

        std::stringstream trackPath;
        trackPath << RESOURCE_PATH << get_string(trackVersion);

        switch (trackVersion) {
        case NFSVersion::NFS_2:
            trackPath << NFS_2_TRACK_PATH << trackName;
            loadedTrack->rawTrack = LibOpenNFS::NFS2::Loader<LibOpenNFS::NFS2::PC>::LoadTrack(trackPath.str(), NFSVersion::NFS_2);
            break;
        case NFSVersion::NFS_2_SE:
            trackPath << NFS_2_SE_TRACK_PATH << trackName;
            loadedTrack->rawTrack = LibOpenNFS::NFS2::Loader<LibOpenNFS::NFS2::PC>::LoadTrack(trackPath.str(), NFSVersion::NFS_2_SE);
            break;
        case NFSVersion::NFS_2_PS1:
            // Somewhat ironically, NFS2 PS1 tracks are more similar to NFS2 PC tracks than NFS3 PS1 tracks in format
            trackPath << "/" << trackName;
            loadedTrack->rawTrack = LibOpenNFS::NFS2::Loader<LibOpenNFS::NFS2::PC>::LoadTrack(trackPath.str(), NFSVersion::NFS_2_PS1);
            break;
        case NFSVersion::NFS_3:
            trackPath << NFS_3_TRACK_PATH << trackName;
            loadedTrack->rawTrack = LibOpenNFS::NFS3::Loader::LoadTrack(trackPath.str());
            break;
        case NFSVersion::NFS_3_PS1:
            trackPath << "/" << trackName;
            loadedTrack->rawTrack = LibOpenNFS::NFS2::Loader<LibOpenNFS::NFS2::PS1>::LoadTrack(trackPath.str(), NFSVersion::NFS_3_PS1);
            break;
            /* case NFS_4:
                 trackPath << NFS_4_TRACK_PATH << trackName;
                 trackData = NFS4::LoadTrack(trackPath.str());
                 nBlocks = std::get<std::shared_ptr<NFS3_4_DATA::TRACK>>(trackData)->nBlocks;
                 cameraAnimations = std::get<std::shared_ptr<NFS3_4_DATA::TRACK>>(trackData)->cameraAnimation;
                 textureArrayID = std::get<std::shared_ptr<NFS3_4_DATA::TRACK>>(trackData)->textureArrayID;
                 trackBlocks = std::get<std::shared_ptr<NFS3_4_DATA::TRACK>>(trackData)->track_blocks;
                 globalObjects = std::get<std::shared_ptr<NFS3_4_DATA::TRACK>>(trackData)->global_objects;
                 break;
             case NFS_4_PS1:
                 trackPath << "/" << trackName << ".GRP";
                 trackData = NFS4PS1::LoadTrack(trackPath.str());
                 ASSERT(false, "Implement!");
                 break;*/
        default:
            ASSERT(false, "Unknown track type!");
        }

        loadedTrack->GenerateSpline();
        loadedTrack->GenerateAabbTree();

        return loadedTrack;
    }
} // namespace OpenNFS
