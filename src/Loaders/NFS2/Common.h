#pragma once

#include <cstdint>
#include <glm/glm.hpp>

struct VERT_HIGHP
{
    int32_t x, z, y;
};

struct ANIM_POS
{
    VERT_HIGHP position;
    int16_t unknown[4];
};