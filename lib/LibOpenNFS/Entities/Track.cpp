#include "Track.h"

namespace LibOpenNFS {
    Track::Track(NFSVersion _nfsVersion, std::string const& _name, std::string const& _basePath) : nfsVersion(_nfsVersion), name(_name), basePath(_basePath) {
    }
} // namespace LibOpenNFS