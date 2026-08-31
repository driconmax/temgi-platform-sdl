#pragma once

#include <temgi/Graphics.h>
#include <temgi/PixelFormat.h>

// Format-aware well-known colors, for engine-internal UI (e.g. the error overlay) that must
// look right on both the indexed and truecolor consoles.
namespace temgi::Colors
{
#if defined(TEMGI_PIXEL_RGB565)
    constexpr Graphics::Pixel Black = 0x0000;
    constexpr Graphics::Pixel White = 0xFFFF;
    constexpr Graphics::Pixel Red   = 0xF800;
#else
    constexpr Graphics::Pixel Black = 0x01; // palette index 1: {0, 0, 0}
    constexpr Graphics::Pixel White = 0xD8; // palette index 216: {255, 255, 255}
    constexpr Graphics::Pixel Red   = 0xB5; // palette index 181: {255, 0, 0}
#endif
}
