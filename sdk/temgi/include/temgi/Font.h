#pragma once

#include <cstdint>

namespace temgi
{
    struct Font {
        std::uint8_t width;
        std::uint8_t height;

        const char* characters;
        std::uint8_t characterCount;

        const std::uint8_t* glyphs;
    };
} // namespace temgi
