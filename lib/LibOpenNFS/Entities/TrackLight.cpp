#include "TrackLight.h"

namespace LibOpenNFS {
    TrackLight::TrackLight(uint32_t entityID, glm::vec3 position, uint32_t nfsType) : BaseLight(entityID, 0u, LightType::TRACK_LIGHT, position, glm::vec4()) {
        // Use Data from NFSHS NFS3 Tracks TR.INI
        switch (nfsType) {
        case 0:
            colour   = glm::vec4(222, 234, 235, 255) / 255.f;
            unknown4 = 5.00f;
            break;
        case 1:
            colour   = glm::vec4(255, 255, 255, 185) / 255.f;
            unknown4 = 4.50f;
            break;
        case 2:
            colour   = glm::vec4(255, 255, 210, 255) / 255.f;
            unknown4 = 5.00f;
            break;
        case 3:
            colour   = glm::vec4(128, 229, 240, 255) / 255.f;
            unknown4 = 4.50f;
            break;
        case 4:
            colour   = glm::vec4(217, 196, 94, 255) / 255.f;
            unknown4 = 5.00f;
            break;
        case 5:
            colour   = glm::vec4(223, 22, 22, 255) / 255.f;
            unknown1 = 1;
            unknown2 = 6;
            unknown3 = 0;
            unknown4 = 5.00f;
            break;
        case 6:
            colour   = glm::vec4(223, 22, 22, 255) / 255.f;
            unknown1 = 1;
            unknown2 = 5;
            unknown3 = 27;
            unknown4 = 5.00f;
            break;
        case 7:
            colour   = glm::vec4(255, 0, 0, 255) / 255.f;
            unknown1 = 1;
            unknown2 = 6;
            unknown3 = 0;
            unknown4 = 3.13f;
            break;
        case 8:
            colour   = glm::vec4(163, 177, 190, 255) / 255.f;
            unknown4 = 3.75f;
            break;
        case 9:
            colour   = glm::vec4(223, 22, 22, 255) / 255.f;
            unknown4 = 3.13f;
            break;
        case 10:
            colour   = glm::vec4(223, 22, 22, 186) / 255.f;
            unknown4 = 2.50f;
            break;
        case 11:
            colour   = glm::vec4(30, 149, 227, 255) / 255.f;
            unknown4 = 2.50f;
            break;
        case 12:
            colour   = glm::vec4(30, 149, 227, 255) / 255.f;
            unknown1 = 1;
            unknown2 = 6;
            unknown3 = 0;
            unknown4 = 3.13f;
            break;
        case 13:
            colour   = glm::vec4(224, 224, 39, 255) / 255.f;
            unknown4 = 3.75f;
            break;
        case 14:
            colour   = glm::vec4(222, 234, 235, 255) / 255.f;
            unknown4 = 5.00f;
            break;
        case 15:
            colour   = glm::vec4(222, 234, 235, 255) / 255.f;
            unknown4 = 5.00f;
            break;
        case 16:
            colour   = glm::vec4(222, 234, 235, 255) / 255.f;
            unknown4 = 5.00f;
            break;
        case 17:
            colour   = glm::vec4(255, 255, 255, 185) / 255.f;
            unknown4 = 4.50f;
            break;
        case 18:
            colour   = glm::vec4(255, 255, 210, 255) / 255.f;
            unknown4 = 5.00f;
            break;
        case 19:
            colour   = glm::vec4(128, 229, 240, 255) / 255.f;
            unknown4 = 4.50f;
            break;
        case 20:
            colour   = glm::vec4(217, 196, 94, 255) / 255.f;
            unknown4 = 5.00f;
            break;
        case 21:
            colour   = glm::vec4(223, 22, 22, 255) / 255.f;
            unknown1 = 1;
            unknown2 = 6;
            unknown3 = 0;
            unknown4 = 5.00f;
            break;
        case 22:
            colour   = glm::vec4(223, 22, 22, 255) / 255.f;
            unknown1 = 1;
            unknown2 = 5;
            unknown3 = 27;
            unknown4 = 5.00f;
            break;
        case 23:
            colour   = glm::vec4(255, 0, 0, 255) / 255.f;
            unknown1 = 1;
            unknown2 = 6;
            unknown3 = 0;
            unknown4 = 3.13f;
            break;
        case 24:
            colour   = glm::vec4(163, 177, 190, 255) / 255.f;
            unknown4 = 3.75f;
            break;
        case 25:
            colour   = glm::vec4(223, 22, 22, 255) / 255.f;
            unknown4 = 3.13f;
            break;
        case 26:
            colour   = glm::vec4(223, 22, 22, 186) / 255.f;
            unknown4 = 2.50f;
            break;
        case 27:
            colour   = glm::vec4(30, 149, 227, 255) / 255.f;
            unknown4 = 2.50f;
            break;
        case 28:
            colour   = glm::vec4(30, 149, 227, 255) / 255.f;
            unknown1 = 1;
            unknown2 = 6;
            unknown3 = 0;
            unknown4 = 3.13f;
            break;
        case 29:
            colour   = glm::vec4(224, 224, 39, 255) / 255.f;
            unknown4 = 3.75f;
            break;
        case 30:
            colour   = glm::vec4(222, 234, 235, 255) / 255.f;
            unknown4 = 5.00f;
            break;
        case 31:
            colour   = glm::vec4(222, 234, 235, 255) / 255.f;
            unknown4 = 5.00f;
            break;
        default:
            colour   = glm::vec4(255, 255, 255, 255) / 255.f;
            unknown4 = 5.00f;
            break;
        }
    }

    TrackLight::TrackLight(uint32_t entityID, glm::vec3 position, glm::vec4 colour, uint32_t unknown1, uint32_t unknown2, uint32_t unknown3, float unknown4) :
        BaseLight(entityID, 0u, LightType::TRACK_LIGHT, position, colour) {
        this->unknown1 = unknown1;
        this->unknown2 = unknown2;
        this->unknown3 = unknown3;
        this->unknown4 = unknown4;
    }
} // namespace LibOpenNFS
