#pragma once

#include "../Scene/Track.h"

class TrackLoader
{
public:
    static std::shared_ptr<Track> LoadTrack(NFSVer trackVersion, const std::string &trackName);
};
