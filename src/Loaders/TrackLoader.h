#pragma once

#include "Track.h"

#include "NFS3/NFS3Loader.h"
/*#include "NFS2/PC/NFS2Loader.h"
#include "NFS4/PC/NFS4Loader.h"
#include "NFS4/PS1/NFS4PS1Loader.h"*/
#include "../nfs_data.h"

class TrackLoader
{
public:
    static std::shared_ptr<Track> LoadTrack(NFSVer trackVersion, const std::string &trackName);
};
