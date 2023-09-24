#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <array>

#include "NFSVersion.h"
#include "Utils.h"

constexpr bool SAFE_READ(std::ifstream &file, void *structure, size_t size) {
    if ((file).read((char *) (structure), (size)).gcount() != (size)) {
        return false;
    }
    return true;
}

static const uint32_t ONFS_SIGNATURE              = 0x15B001C0;
const std::array<uint8_t, 6> quadToTriVertNumbers = {0, 1, 2, 0, 2, 3};

class IRawData {
protected:
    virtual bool _SerializeIn(std::ifstream &ifstream)  = 0;
    virtual void _SerializeOut(std::ofstream &ofstream) = 0;
};
