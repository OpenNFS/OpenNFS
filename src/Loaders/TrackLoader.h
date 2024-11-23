#pragma once

#include "../Scene/Track.h"

namespace OpenNFS {
    class TrackLoader {
    public:
        static std::shared_ptr<OpenNFS::Track> Load(NFSVersion nfsVersion, const std::string &trackName);
    };
} // namespace OpenNFS
