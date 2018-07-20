//
// Created by Amrik.Sadhra on 20/06/2018.
//

#pragma once

#include <memory>
#include "../nfs_data.h"
#include "nfs3_loader.h"
#include "nfs2_loader.h"
#include "nfs4_loader.h"
#include <boost/variant.hpp>

class ONFSTrack {
public:
    explicit ONFSTrack(const std::string &track_path);
    ~ONFSTrack() {}

    NFSVer tag;
    typedef boost::variant<shared_ptr<NFS3_4_DATA::TRACK>, shared_ptr<NFS2_DATA::PS1::TRACK>, shared_ptr<NFS2_DATA::PC::TRACK>> track;
    track trackData;

    std::vector<TrackBlock> track_blocks;
    uint32_t nBlocks;
    map<short, GLuint> texture_gl_mappings;
};

class TrackLoader {
public:
    static shared_ptr<ONFSTrack> LoadTrack(const std::string &track_path);
};