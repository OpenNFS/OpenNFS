#include "TrackLoader.h"

#include "../Util/Utils.h"
#include "NFS3/NFS3Loader.h"
#include "NFS2/NFS2Loader.h"
/*#include "NFS4/PC/NFS4Loader.h"
#include "NFS4/PS1/NFS4PS1Loader.h"*/

namespace OpenNFS {
    std::shared_ptr<Track> TrackLoader::Load(NFSVersion const nfsVersion, const std::string &trackName) {
        std::stringstream trackBasePath, trackOutPath;
        trackBasePath << RESOURCE_PATH << get_string(nfsVersion);
        trackOutPath << TRACK_PATH << get_string(nfsVersion) << "/";

        switch (nfsVersion) {
        case NFSVersion::NFS_2:
            trackBasePath << NFS_2_TRACK_PATH << trackName;
            return std::make_shared<Track>(LibOpenNFS::NFS2::Loader<LibOpenNFS::NFS2::PC>::LoadTrack(NFSVersion::NFS_2, trackBasePath.str(),trackOutPath.str()));
        case NFSVersion::NFS_2_SE:
            trackBasePath << NFS_2_SE_TRACK_PATH << trackName;
            return std::make_shared<Track>(LibOpenNFS::NFS2::Loader<LibOpenNFS::NFS2::PC>::LoadTrack(NFSVersion::NFS_2_SE, trackBasePath.str(), trackOutPath.str()));
        case NFSVersion::NFS_2_PS1:
            // Somewhat ironically, NFS2 PS1 tracks are more similar to NFS2 PC tracks than NFS3 PS1 tracks in format
            trackBasePath << "/" << trackName;
            return std::make_shared<Track>(LibOpenNFS::NFS2::Loader<LibOpenNFS::NFS2::PC>::LoadTrack(NFSVersion::NFS_2_PS1, trackBasePath.str(), trackOutPath.str()));
        case NFSVersion::NFS_3:
            trackBasePath << NFS_3_TRACK_PATH << trackName;
            return std::make_shared<Track>(LibOpenNFS::NFS3::Loader::LoadTrack(trackBasePath.str(), trackOutPath.str()));
        case NFSVersion::NFS_3_PS1:
            trackBasePath << "/" << trackName;
            return std::make_shared<Track>(LibOpenNFS::NFS2::Loader<LibOpenNFS::NFS2::PS1>::LoadTrack(NFSVersion::NFS_3_PS1, trackBasePath.str(),trackOutPath.str() ));
            /* case NFSVersion::NFS_4:
                 trackPath << NFS_4_TRACK_PATH << trackName;
                 trackData = NFS4::LoadTrack(trackPath.str());
                 nBlocks = std::get<std::shared_ptr<NFS3_4_DATA::TRACK>>(trackData)->nBlocks;
                 cameraAnimations = std::get<std::shared_ptr<NFS3_4_DATA::TRACK>>(trackData)->cameraAnimation;
                 textureArrayID = std::get<std::shared_ptr<NFS3_4_DATA::TRACK>>(trackData)->textureArrayID;
                 trackBlocks = std::get<std::shared_ptr<NFS3_4_DATA::TRACK>>(trackData)->track_blocks;
                 globalObjects = std::get<std::shared_ptr<NFS3_4_DATA::TRACK>>(trackData)->global_objects;
                 break;
             case NFSVersion::NFS_4_PS1:
                 trackPath << "/" << trackName << ".GRP";
                 trackData = NFS4PS1::LoadTrack(trackPath.str());
                 CHECK_F(false, "Implement!");
                 break;*/
        default:
            CHECK_F(false, "Unknown track type!");
        }
       unreachable();
    }
} // namespace OpenNFS
