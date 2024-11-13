#include "Track.h"

namespace LibOpenNFS {
    Track::Track(const Track& track) {
        this->nfsVersion      = track.nfsVersion;
        this->name            = track.name;
        this->nBlocks         = track.nBlocks;
        this->cameraAnimation = track.cameraAnimation;
        this->virtualRoad     = track.virtualRoad;
        this->trackBlocks     = track.trackBlocks;
        this->globalObjects   = track.globalObjects;
        this->vroadBarriers   = track.vroadBarriers;
    }

    Track::Track(NFSVersion _nfsVersion, std::string const& _name) : nfsVersion(_nfsVersion), name(_name) {
    }
} // namespace LibOpenNFS