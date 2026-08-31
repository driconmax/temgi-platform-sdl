#pragma once

#include <cstdint>

#include <temgi/PixelFormat.h>

namespace temgi
{
    struct Animation
    {
        std::uint16_t width;
        std::uint16_t height;
        std::uint16_t frameCount;

        const PixelStorage* pixels;
    };
}