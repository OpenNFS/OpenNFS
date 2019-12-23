#pragma once

#include <memory>
#include <boost/variant.hpp>

#include "NFS3Loader.h"
#include "NFS2Loader.h"
#include "NFS4Loader.h"
#include "NFS4PS1Loader.h"
#include "../nfs_data.h"
#include "../Renderer/HermiteCurve.h"

class ONFSTrack {
public:
    explicit ONFSTrack(NFSVer trackVersion, const std::string &trackName);

    ~ONFSTrack() {
        switch (tag) {
            case NFS_2:
            case NFS_2_SE:
            case NFS_3_PS1:
            case NFS_4:
            case NFS_3:
                // NFS3::FreeTrack(boost::get<std::shared_ptr<NFS3_4_DATA::TRACK>>(trackData));
                break;
            case UNKNOWN:
                ASSERT(false, "Unknown track type!");
            default:
                break;
        }
    }

    NFSVer tag;
    std::string name;
    typedef boost::variant<std::shared_ptr<NFS3_4_DATA::TRACK>, std::shared_ptr<NFS3_4_DATA::PS1::TRACK>, std::shared_ptr<NFS2_DATA::PS1::TRACK>, std::shared_ptr<NFS2_DATA::PC::TRACK>> track;
    track trackData;

    std::vector<SHARED::CANPT> cameraAnimations;
    std::vector<TrackBlock> trackBlocks;
    std::vector<Entity> globalObjects;
    std::vector<Entity> vroadBarriers;
    uint32_t nBlocks;
    GLuint textureArrayID;
    HermiteCurve centerSpline;

private:
    void _GenerateSpline();
    void _GenerateAabbTree();
};

class TrackLoader {
public:
    static std::shared_ptr<ONFSTrack> LoadTrack(NFSVer nfs_version, const std::string &track_name);
};