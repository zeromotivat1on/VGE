#pragma once

#include <fstream>
#include <string>
#include <vector>
#include "Types.h"

namespace vge::fs
{
    std::string GetExtension(const std::string& uri);
    
    std::vector<u8> ReadAsset(const std::string& filename, u32 count = 0);
}
