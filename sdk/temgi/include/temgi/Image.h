#pragma once

#include <cstdint>

namespace temgi
{
    struct Image
    {
        std::uint16_t width;
        std::uint16_t height;

        const std::uint8_t* pixels;
    };
}