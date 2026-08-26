#pragma once

#include <cstdint>

namespace temgi
{
    struct Animation
    {
        std::uint16_t width;
        std::uint16_t height;
        std::uint16_t frameCount;

        const std::uint8_t* pixels;
    };
}