#pragma once

#include "BaseLight.h"

#include <memory>

namespace LibOpenNFS {
    class TrackLight : public BaseLight {
    public:
        TrackLight(uint32_t entityID, glm::vec3 position, uint32_t nfsType);
        TrackLight(uint32_t entityID, glm::vec3 position, glm::vec4 colour, uint32_t unknown1, uint32_t unknown2, uint32_t unknown3, float unknown4);

        // NFS3 and 4 light data stored in TR.ini [track glows]
        uint32_t nfsType;
        uint32_t unknown1, unknown2, unknown3;
        float unknown4;
    };
} // namespace LibOpenNFS
