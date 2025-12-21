#pragma once

#include "../Scene/Track.h"

namespace OpenNFS {
    class TrackLoader {
      public:
        static Track Load(NFSVersion nfsVersion, std::string const &trackName);
    };
} // namespace OpenNFS
