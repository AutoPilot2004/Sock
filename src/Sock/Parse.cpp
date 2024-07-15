#include "Parse.h"

#include <cctype>
#include <stdexcept>

namespace
{
    int char2hex(char chr)
    {
        char tlchr = std::tolower(chr);

        if (tlchr >= '0' && tlchr <= '9') return tlchr - '0';
        if (tlchr >= 'a' && tlchr <= 'f') return tlchr - 'a' + 10;

        throw std::logic_error("Character passed is not hex!");
    }

    // - Only byte[0] and byte[1] are used
    // - If the pointer is not pointing to a char array with size >= 2,
    //   it's UB
    int parse_hexbyte(const char* byte)
    {
        constexpr uint8_t HEX_BASE = 16;

        return (char2hex(byte[0]) * HEX_BASE) + char2hex(byte[1]);
    }

    bool valid_mac_sep(char chr)
    {
        return chr == ':' || chr == '-';
    }
}

namespace Sock
{
    MacAddr parse_mac(std::string_view mac)
    {
        constexpr uint8_t RAW_MAC_LEN = 12;
        constexpr uint8_t SEP_MAC_LEN = 17;

        const size_t size = mac.size();

        bool isRaw = (size == RAW_MAC_LEN);
        bool isWithSep = (size == SEP_MAC_LEN);

        char sep = mac.at(2);

        if (!isRaw) {
            if (!isWithSep)          throw std::runtime_error("Invalid MAC Address size!");
            if (!valid_mac_sep(sep)) throw std::runtime_error("Invalid MAC Address separators!");
        }

        uint8_t step = (isRaw * 2) + (isWithSep * 3);

        MacAddr ma;

        for (int i = 0, idx = 0; i < size; i += step, idx++) {
            if (i && isWithSep && mac.at(i - 1) != sep) throw std::runtime_error("Invalid MAC Address!");
            ma.byte[idx] = parse_hexbyte(&mac.at(i));
        }

        return ma;
    }
}