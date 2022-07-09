#include "TrackUtils.h"

namespace TrackUtils {
    std::shared_ptr<TrackLight> MakeLight(glm::vec3 position, uint32_t nfsType) {
        // Use Data from NFSHS NFS3 Tracks TR.INI
        switch (nfsType) {
        case 0:
            return std::make_shared<TrackLight>(position, glm::vec4(222, 234, 235, 255) / 255.f, nfsType, 0, 0, 0, 5.00f);
        case 1:
            return std::make_shared<TrackLight>(position, glm::vec4(255, 255, 255, 185) / 255.f, nfsType, 0, 0, 0, 4.50f);
        case 2:
            return std::make_shared<TrackLight>(position, glm::vec4(255, 255, 210, 255) / 255.f, nfsType, 0, 0, 0, 5.00f);
        case 3:
            return std::make_shared<TrackLight>(position, glm::vec4(128, 229, 240, 255) / 255.f, nfsType, 0, 0, 0, 4.50f);
        case 4:
            return std::make_shared<TrackLight>(position, glm::vec4(217, 196, 94, 255) / 255.f, nfsType, 0, 0, 0, 5.00f);
        case 5:
            return std::make_shared<TrackLight>(position, glm::vec4(223, 22, 22, 255) / 255.f, nfsType, 1, 6, 0, 5.00f);
        case 6:
            return std::make_shared<TrackLight>(position, glm::vec4(223, 22, 22, 255) / 255.f, nfsType, 1, 5, 27, 5.00f);
        case 7:
            return std::make_shared<TrackLight>(position, glm::vec4(255, 0, 0, 255) / 255.f, nfsType, 1, 6, 0, 3.13f);
        case 8:
            return std::make_shared<TrackLight>(position, glm::vec4(163, 177, 190, 255) / 255.f, nfsType, 0, 0, 0, 3.75f);
        case 9:
            return std::make_shared<TrackLight>(position, glm::vec4(223, 22, 22, 255) / 255.f, nfsType, 0, 0, 0, 3.13f);
        case 10:
            return std::make_shared<TrackLight>(position, glm::vec4(223, 22, 22, 186) / 255.f, nfsType, 0, 0, 0, 2.50f);
        case 11:
            return std::make_shared<TrackLight>(position, glm::vec4(30, 149, 227, 255) / 255.f, nfsType, 0, 0, 0, 2.50f);
        case 12:
            return std::make_shared<TrackLight>(position, glm::vec4(30, 149, 227, 255) / 255.f, nfsType, 1, 6, 0, 3.13f);
        case 13:
            return std::make_shared<TrackLight>(position, glm::vec4(224, 224, 39, 255) / 255.f, nfsType, 0, 0, 0, 3.75f);
        case 14:
            return std::make_shared<TrackLight>(position, glm::vec4(222, 234, 235, 255) / 255.f, nfsType, 0, 0, 0, 5.00f);
        case 15:
            return std::make_shared<TrackLight>(position, glm::vec4(222, 234, 235, 255) / 255.f, nfsType, 0, 0, 0, 5.00f);
        case 16:
            return std::make_shared<TrackLight>(position, glm::vec4(222, 234, 235, 255) / 255.f, nfsType, 0, 0, 0, 5.00f);
        case 17:
            return std::make_shared<TrackLight>(position, glm::vec4(255, 255, 255, 185) / 255.f, nfsType, 0, 0, 0, 4.50f);
        case 18:
            return std::make_shared<TrackLight>(position, glm::vec4(255, 255, 210, 255) / 255.f, nfsType, 0, 0, 0, 5.00f);
        case 19:
            return std::make_shared<TrackLight>(position, glm::vec4(128, 229, 240, 255) / 255.f, nfsType, 0, 0, 0, 4.50f);
        case 20:
            return std::make_shared<TrackLight>(position, glm::vec4(217, 196, 94, 255) / 255.f, nfsType, 0, 0, 0, 5.00f);
        case 21:
            return std::make_shared<TrackLight>(position, glm::vec4(223, 22, 22, 255) / 255.f, nfsType, 1, 6, 0, 5.00f);
        case 22:
            return std::make_shared<TrackLight>(position, glm::vec4(223, 22, 22, 255) / 255.f, nfsType, 1, 5, 27, 5.00f);
        case 23:
            return std::make_shared<TrackLight>(position, glm::vec4(255, 0, 0, 255) / 255.f, nfsType, 1, 6, 0, 3.13f);
        case 24:
            return std::make_shared<TrackLight>(position, glm::vec4(163, 177, 190, 255) / 255.f, nfsType, 0, 0, 0, 3.75f);
        case 25:
            return std::make_shared<TrackLight>(position, glm::vec4(223, 22, 22, 255) / 255.f, nfsType, 0, 0, 0, 3.13f);
        case 26:
            return std::make_shared<TrackLight>(position, glm::vec4(223, 22, 22, 186) / 255.f, nfsType, 0, 0, 0, 2.50f);
        case 27:
            return std::make_shared<TrackLight>(position, glm::vec4(30, 149, 227, 255) / 255.f, nfsType, 0, 0, 0, 2.50f);
        case 28:
            return std::make_shared<TrackLight>(position, glm::vec4(30, 149, 227, 255) / 255.f, nfsType, 1, 6, 0, 3.13f);
        case 29:
            return std::make_shared<TrackLight>(position, glm::vec4(224, 224, 39, 255) / 255.f, nfsType, 0, 0, 0, 3.75f);
        case 30:
            return std::make_shared<TrackLight>(position, glm::vec4(222, 234, 235, 255) / 255.f, nfsType, 0, 0, 0, 5.00f);
        case 31:
            return std::make_shared<TrackLight>(position, glm::vec4(222, 234, 235, 255) / 255.f, nfsType, 0, 0, 0, 5.00f);
        default:
            return std::make_shared<TrackLight>(position, glm::vec4(255, 255, 255, 255) / 255.f, nfsType, 0, 0, 0, 5.00f);
        }
    }

    // Break Packed uint32_t RGBA per vertex colour data for baked lighting of RGB into 4 normalised floats and store into vec4
    glm::vec4 ShadingDataToVec4(uint32_t packedRgba) {
        return glm::vec4(((packedRgba >> 16) & 0xFF) / 255.0f, ((packedRgba >> 8) & 0xFF) / 255.0f, (packedRgba & 0xFF) / 255.0f, ((packedRgba >> 24) & 0xFF) / 255.0f);
    }

    glm::vec4 ShadingDataToVec4(uint16_t packedRgba) {
        // BGR565
        // return glm::vec4(((packedRgba >> 11) & 0x1F) / 32.0f, ((packedRgba >> 5) & 0x3F) / 64.0f, ((packedRgba ) & 0x1F) / 32.0f, 0.2f);
        // RGB565 (best, but blueish)
        // return glm::vec4(((packedRgba) & 0x1F) / 32.0f, ((packedRgba >> 5) & 0x3F) / 64.0f, ((packedRgba >> 11 ) & 0x1F) / 32.0f, 1.0f);
        // ARGB 4444
        // return glm::vec4(((packedRgba >> 8) & 0xF) / 16.0f, ((packedRgba >> 4) & 0xF) / 16.0f, ((packedRgba) & 0xF) / 16.0f, ((packedRgba >> 12) & 0xF) / 16.0f);
        // BGRA 4444
        // return glm::vec4(((packedRgba >> 4) & 0xF) / 16.0f, ((packedRgba >> 8) & 0xF) / 16.0f, ((packedRgba >> 12) & 0xF) / 16.0f, ((packedRgba) & 0xF) / 16.0f);
        // RGBA 4444
        // return glm::vec4(((packedRgba >> 12) & 0xF) / 16.0f, ((packedRgba >> 8) & 0xF) / 16.0f, ((packedRgba >> 4) & 0xF) / 16.0f, ((packedRgba) & 0xF) / 16.0f);
        // RGBA 5551
        // return glm::vec4(((packedRgba >> 11) & 0x1F) / 32.0f, ((packedRgba >> 6) & 0x1F) / 32.0f, ((packedRgba >> 1) & 0x1F) / 32.0f, ((packedRgba) & 0x1));
        // They're all broken lol, return nothing
        return glm::vec4(1.f, 1.f, 1.f, 1.f);
    }
} // namespace TrackUtils
