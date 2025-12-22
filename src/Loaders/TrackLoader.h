#pragma once

#include "../Scene/Track.h"

namespace OpenNFS {
    class TrackLoader {
      public:
        static std::shared_ptr<Track> Load(NFSVersion nfsVersion, std::string const &trackName);
    };
} // namespace OpenNFS
