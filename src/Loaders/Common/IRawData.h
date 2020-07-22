#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <cstdint>

#include "../NFS3/Common.h"
#include "../../Util/Utils.h"

static const uint32_t ONFS_SIGNATURE = 0x15B001C0;

class IRawData
{
protected:
    virtual bool _SerializeIn(std::ifstream &ifstream)  = 0;
    virtual void _SerializeOut(std::ofstream &ofstream) = 0;
};
