//
// Created by Amrik.Sadhra on 20/06/2018.
//

#include "trk_loader.h"

ONFSTrack::ONFSTrack(NFSVer nfs_version, const std::string &track_name) {
    tag = nfs_version;
    name = track_name;

    std::stringstream track_path;
    track_path << RESOURCE_PATH << ToString(tag);

    switch (tag) {
        case NFS_2:
            track_path << NFS_2_TRACK_PATH << track_name;
            trackData = NFS2<PC>::LoadTrack(track_path.str());
            nBlocks = boost::get<shared_ptr<NFS2_DATA::PC::TRACK>>(trackData)->nBlocks;
            texture_array = boost::get<shared_ptr<NFS2_DATA::PC::TRACK>>(trackData)->texture_array;
            track_blocks = boost::get<shared_ptr<NFS2_DATA::PC::TRACK>>(trackData)->track_blocks;
            global_objects = boost::get<shared_ptr<NFS2_DATA::PC::TRACK>>(trackData)->global_objects;
            break;
        case NFS_2_SE:
            track_path << NFS_2_SE_TRACK_PATH << track_name;
            trackData = NFS2<PC>::LoadTrack(track_path.str());
            nBlocks = boost::get<shared_ptr<NFS2_DATA::PC::TRACK>>(trackData)->nBlocks;
            texture_array = boost::get<shared_ptr<NFS2_DATA::PC::TRACK>>(trackData)->texture_array;
            track_blocks = boost::get<shared_ptr<NFS2_DATA::PC::TRACK>>(trackData)->track_blocks;
            global_objects = boost::get<shared_ptr<NFS2_DATA::PC::TRACK>>(trackData)->global_objects;
            break;
        case NFS_3:
            track_path << NFS_3_TRACK_PATH << track_name;
            trackData = NFS3::LoadTrack(track_path.str());
            nBlocks = boost::get<shared_ptr<NFS3_4_DATA::TRACK>>(trackData)->nBlocks;
            texture_array =  boost::get<shared_ptr<NFS3_4_DATA::TRACK>>(trackData)->texture_array;
            track_blocks = boost::get<shared_ptr<NFS3_4_DATA::TRACK>>(trackData)->track_blocks;
            global_objects = boost::get<shared_ptr<NFS3_4_DATA::TRACK>>(trackData)->global_objects;
            break;
        case NFS_3_PS1:
            track_path << "/" << track_name;
            trackData = NFS2<PS1>::LoadTrack(track_path.str());
            nBlocks = boost::get<shared_ptr<NFS2_DATA::PS1::TRACK>>(trackData)->nBlocks;
            texture_array =  boost::get<shared_ptr<NFS2_DATA::PS1::TRACK>>(trackData)->texture_array;
            track_blocks = boost::get<shared_ptr<NFS2_DATA::PS1::TRACK>>(trackData)->track_blocks;
            global_objects = boost::get<shared_ptr<NFS2_DATA::PS1::TRACK>>(trackData)->global_objects;
            break;
        case NFS_4:
            track_path << NFS_4_TRACK_PATH << track_name;
            trackData = NFS4::LoadTrack(track_path.str());
            nBlocks = boost::get<shared_ptr<NFS3_4_DATA::TRACK>>(trackData)->nBlocks;
            texture_array = boost::get<shared_ptr<NFS3_4_DATA::TRACK>>(trackData)->texture_array;
            track_blocks = boost::get<shared_ptr<NFS3_4_DATA::TRACK>>(trackData)->track_blocks;
            break;
        case UNKNOWN:
            ASSERT(false, "Unknown track type!");
        default:
            break;
    }
}

shared_ptr<ONFSTrack> TrackLoader::LoadTrack(NFSVer nfs_version, const std::string &track_name) {
    return std::shared_ptr<ONFSTrack>(new ONFSTrack(nfs_version, track_name));
}
