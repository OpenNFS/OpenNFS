#pragma once

#include "../Scene/Track.h"

namespace OpenNFS {
    class TrackLoader {
    public:
        static std::shared_ptr<OpenNFS::Track> LoadTrack(NFSVersion trackVersion, const std::string &trackName);
    };
} // namespace OpenNFS
