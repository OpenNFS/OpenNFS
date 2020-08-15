#include "TrackLight.h"

TrackLight::TrackLight(glm::vec3 position, glm::vec4 colour, uint32_t nfsType, uint32_t unknown1, uint32_t unknown2, uint32_t unknown3, float unknown4) :
    BaseLight(LightType::TRACK_LIGHT, position, colour)
{
    this->nfsType  = nfsType;
    this->unknown1 = unknown1;
    this->unknown2 = unknown2;
    this->unknown3 = unknown3;
    this->unknown4 = unknown4;
}
