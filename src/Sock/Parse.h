#pragma once

#include <cstdint>
#include <string_view>

#include "Socket/Socket.h"

namespace Sock
{   
    MacAddr parse_mac(std::string_view mac);
}