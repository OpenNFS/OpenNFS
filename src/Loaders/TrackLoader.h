#pragma once

#include "../Scene/Track.h"

namespace OpenNFS {
    class TrackLoader {
    public:
        static Track Load(NFSVersion nfsVersion, const std::string &trackName);
    };
} // namespace OpenNFS
