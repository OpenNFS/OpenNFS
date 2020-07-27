#pragma once

#include <cstdint>
#include <glm/glm.hpp>

namespace LibOpenNFS
{
    namespace NFS2
    {
        struct VERT_HIGHP
        {
            int32_t x, z, y;
        };

        struct ANIM_POS
        {
            VERT_HIGHP position;
            int16_t unknown[4];
        };

        struct PC
        {
            struct VERT
            {
                int16_t x, z, y;
            };

            struct POLYGONDATA
            {
                int16_t texture;
                int16_t otherSideTex;
                uint8_t vertex[4];
            };
        };

        struct PS1
        {
            struct VERT
            {
                int16_t x, z, y, w;
            };

            struct POLYGONDATA
            {
                uint8_t texture;
                uint8_t otherSideTex;
                uint8_t vertex[4];
            };
        };
    } // namespace NFS2
} // namespace LibOpenNFS
