#include "TrackLoader.h"

#include "../Util/Utils.h"
#include "NFS2/NFS2Loader.h"
#include "NFS3/NFS3Loader.h"
#include "NFS4/PC/NFS4Loader.h"

namespace OpenNFS {
    using namespace LibOpenNFS;

    Track TrackLoader::Load(NFSVersion const nfsVersion, std::string const &trackName) {
        std::stringstream trackBasePath, trackOutPath;
        trackBasePath << RESOURCE_PATH << magic_enum::enum_name(nfsVersion);
        trackOutPath << TRACK_PATH << magic_enum::enum_name(nfsVersion) << "/";

        switch (nfsVersion) {
        case NFSVersion::NFS_2:
            trackBasePath << NFS_2_TRACK_PATH << trackName;
            return OpenNFS::Track(NFS2::Loader<NFS2::PC>::LoadTrack(NFSVersion::NFS_2, trackBasePath.str(), trackOutPath.str()));
        case NFSVersion::NFS_2_SE:
            trackBasePath << NFS_2_SE_TRACK_PATH << trackName;
            return OpenNFS::Track(NFS2::Loader<NFS2::PC>::LoadTrack(NFSVersion::NFS_2_SE, trackBasePath.str(), trackOutPath.str()));
        case NFSVersion::NFS_2_PS1:
            // Somewhat ironically, NFS2 PS1 tracks are more similar to NFS2 PC tracks than NFS3 PS1 tracks in format
            trackBasePath << "/" << trackName;
            return OpenNFS::Track(NFS2::Loader<NFS2::PC>::LoadTrack(NFSVersion::NFS_2_PS1, trackBasePath.str(), trackOutPath.str()));
        case NFSVersion::NFS_3:
            trackBasePath << NFS_3_TRACK_PATH << trackName;
            return OpenNFS::Track(NFS3::Loader::LoadTrack(trackBasePath.str(), trackOutPath.str()));
        case NFSVersion::NFS_3_PS1:
            trackBasePath << "/" << trackName;
            return OpenNFS::Track(NFS2::Loader<NFS2::PS1>::LoadTrack(NFSVersion::NFS_3_PS1, trackBasePath.str(), trackOutPath.str()));
        case NFSVersion::NFS_4:
            trackBasePath << NFS_4_TRACK_PATH << trackName;
            return OpenNFS::Track(NFS4::Loader::LoadTrack(trackBasePath.str(), trackOutPath.str()));
        default:
            CHECK_F(false, "Unknown track type!");
        }
        unreachable();
    }
} // namespace OpenNFS
