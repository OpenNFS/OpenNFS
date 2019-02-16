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
            camera_animations = boost::get<shared_ptr<NFS2_DATA::PC::TRACK>>(trackData)->cameraAnimation;
            textureArrayID = boost::get<shared_ptr<NFS2_DATA::PC::TRACK>>(trackData)->textureArrayID;
            track_blocks = boost::get<shared_ptr<NFS2_DATA::PC::TRACK>>(trackData)->track_blocks;
            global_objects = boost::get<shared_ptr<NFS2_DATA::PC::TRACK>>(trackData)->global_objects;
            break;
        case NFS_2_SE:
            track_path << NFS_2_SE_TRACK_PATH << track_name;
            trackData = NFS2<PC>::LoadTrack(track_path.str());
            nBlocks = boost::get<shared_ptr<NFS2_DATA::PC::TRACK>>(trackData)->nBlocks;
            camera_animations = boost::get<shared_ptr<NFS2_DATA::PC::TRACK>>(trackData)->cameraAnimation;
            textureArrayID = boost::get<shared_ptr<NFS2_DATA::PC::TRACK>>(trackData)->textureArrayID;
            track_blocks = boost::get<shared_ptr<NFS2_DATA::PC::TRACK>>(trackData)->track_blocks;
            global_objects = boost::get<shared_ptr<NFS2_DATA::PC::TRACK>>(trackData)->global_objects;
            break;
        case NFS_3:
            // TODO: Remove this code that tests FFN Load
            //NFS3::LoadFFN("G:/NFS3/nfs3_modern_base_eng_font_fuckery/fedata/text/sci20.ffn.ori");
            track_path << NFS_3_TRACK_PATH << track_name;
            trackData = NFS3::LoadTrack(track_path.str());
            nBlocks = boost::get<shared_ptr<NFS3_4_DATA::TRACK>>(trackData)->nBlocks;
            camera_animations = boost::get<shared_ptr<NFS3_4_DATA::TRACK>>(trackData)->cameraAnimation;
            textureArrayID =  boost::get<shared_ptr<NFS3_4_DATA::TRACK>>(trackData)->textureArrayID;
            track_blocks = boost::get<shared_ptr<NFS3_4_DATA::TRACK>>(trackData)->track_blocks;
            global_objects = boost::get<shared_ptr<NFS3_4_DATA::TRACK>>(trackData)->global_objects;
            break;
        case NFS_3_PS1:
            track_path << "/" << track_name;
            trackData = NFS2<PS1>::LoadTrack(track_path.str());
            nBlocks = boost::get<shared_ptr<NFS2_DATA::PS1::TRACK>>(trackData)->nBlocks;
            camera_animations = boost::get<shared_ptr<NFS2_DATA::PS1::TRACK>>(trackData)->cameraAnimation;
            textureArrayID =  boost::get<shared_ptr<NFS2_DATA::PS1::TRACK>>(trackData)->textureArrayID;
            track_blocks = boost::get<shared_ptr<NFS2_DATA::PS1::TRACK>>(trackData)->track_blocks;
            global_objects = boost::get<shared_ptr<NFS2_DATA::PS1::TRACK>>(trackData)->global_objects;
            break;
        case NFS_4:
            track_path << NFS_4_TRACK_PATH << track_name;
            trackData = NFS4::LoadTrack(track_path.str());
            nBlocks = boost::get<shared_ptr<NFS3_4_DATA::TRACK>>(trackData)->nBlocks;
            camera_animations = boost::get<shared_ptr<NFS3_4_DATA::TRACK>>(trackData)->cameraAnimation;
            textureArrayID = boost::get<shared_ptr<NFS3_4_DATA::TRACK>>(trackData)->textureArrayID;
            track_blocks = boost::get<shared_ptr<NFS3_4_DATA::TRACK>>(trackData)->track_blocks;
            global_objects = boost::get<shared_ptr<NFS3_4_DATA::TRACK>>(trackData)->global_objects;
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
