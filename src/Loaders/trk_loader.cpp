//
// Created by Amrik.Sadhra on 20/06/2018.
//

#include "trk_loader.h"

ONFSTrack::ONFSTrack(const std::string &track_path) {
    // Do some custom NFS detection logic (realistically just check the path for the enums)
    if (track_path.find("NFS2_SE") != std::string::npos) {
        tag = NFS_2_SE;
    } else if (track_path.find("NFS2") != std::string::npos) {
        tag = NFS_2;
    } else if (track_path.find("NFS3") != std::string::npos) {
        tag = NFS_3;
    } else if (track_path.find("NFS3_PS1") != std::string::npos) {
        tag = NFS_3_PS1;
    } else if (track_path.find("NFS4") != std::string::npos) {
        tag = NFS_4;
    } else {
        tag = UNKNOWN;
    }

    switch (tag) {
        case NFS_2:
            trackData = NFS2<PC>::LoadTrack(track_path);
            nBlocks = boost::get<shared_ptr<NFS2_DATA::PC::TRACK>>(trackData)->nBlocks;
            texture_gl_mappings = boost::get<shared_ptr<NFS2_DATA::PC::TRACK>>(trackData)->texture_gl_mappings;
            track_blocks = boost::get<shared_ptr<NFS2_DATA::PC::TRACK>>(trackData)->track_blocks;
            break;
        case NFS_2_SE:
            trackData = NFS2<PC>::LoadTrack(track_path);
            nBlocks = boost::get<shared_ptr<NFS2_DATA::PS1::TRACK>>(trackData)->nBlocks;
            texture_gl_mappings = boost::get<shared_ptr<NFS2_DATA::PS1::TRACK>>(trackData)->texture_gl_mappings;
            track_blocks = boost::get<shared_ptr<NFS2_DATA::PS1::TRACK>>(trackData)->track_blocks;
            break;
        case NFS_3:
            trackData = NFS3::LoadTrack(track_path);
            nBlocks = boost::get<shared_ptr<NFS3_4_DATA::TRACK>>(trackData)->nBlocks;
            texture_gl_mappings = boost::get<shared_ptr<NFS3_4_DATA::TRACK>>(trackData)->texture_gl_mappings;
            track_blocks = boost::get<shared_ptr<NFS3_4_DATA::TRACK>>(trackData)->track_blocks;
            break;
        case NFS_3_PS1:
            trackData = NFS2<PS1>::LoadTrack(track_path);
            nBlocks = boost::get<shared_ptr<NFS2_DATA::PS1::TRACK>>(trackData)->nBlocks;
            texture_gl_mappings = boost::get<shared_ptr<NFS2_DATA::PS1::TRACK>>(trackData)->texture_gl_mappings;
            track_blocks = boost::get<shared_ptr<NFS2_DATA::PS1::TRACK>>(trackData)->track_blocks;
            break;
        case NFS_4:
            trackData = NFS4::LoadTrack(track_path);
            nBlocks = boost::get<shared_ptr<NFS3_4_DATA::TRACK>>(trackData)->nBlocks;
            texture_gl_mappings = boost::get<shared_ptr<NFS3_4_DATA::TRACK>>(trackData)->texture_gl_mappings;
            track_blocks = boost::get<shared_ptr<NFS3_4_DATA::TRACK>>(trackData)->track_blocks;
            break;
        case UNKNOWN:
            ASSERT(false, "Unknown track type!");
        default:
            break;
    }
}

shared_ptr<ONFSTrack> TrackLoader::LoadTrack(const std::string &track_path){
    return std::shared_ptr<ONFSTrack>(new ONFSTrack("../resources/NFS3/gamedata/tracks/trk006/tr06"));
}
