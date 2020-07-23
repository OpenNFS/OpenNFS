#pragma once

#include <memory>
#include "../Scene/Lights/TrackLight.h"

namespace TrackUtils
{
    std::shared_ptr<TrackLight> MakeLight(glm::vec3 position, uint32_t nfsType);
    glm::vec4 ShadingDataToVec4(uint32_t packedRgba);
}; // namespace TrackUtils
