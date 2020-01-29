#pragma once

#include "BaseLight.h"
#include "LightModel.h"

class TrackLight : public BaseLight
{
public:
    TrackLight(glm::vec3 position, glm::vec4 colour, uint32_t nfsType, uint32_t unknown1, uint32_t unknown2, uint32_t unknown3, float unknown4);

    LightModel model;
    // NFS3 and 4 light data stored in TR.ini [track glows]
    uint32_t nfsType;
    uint32_t unknown1, unknown2, unknown3;
    float unknown4;
};
