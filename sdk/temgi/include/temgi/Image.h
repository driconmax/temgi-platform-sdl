#pragma once

#include <cstdint>

#include <temgi/PixelFormat.h>

namespace temgi
{
    struct Image
    {
        std::uint16_t width;
        std::uint16_t height;

        const PixelStorage* pixels;
    };
}