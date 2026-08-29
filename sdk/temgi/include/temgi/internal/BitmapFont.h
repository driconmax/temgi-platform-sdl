#pragma once

#include <array>
#include <cstdint>

namespace temgi
{
    class BitmapFont {
        public:
            static constexpr std::uint8_t WIDTH = 5;
            static constexpr std::uint8_t HEIGHT = 7;

            using Glyph = std::array<std::uint8_t, HEIGHT>;

            static const Glyph& glyph(char character);
    };
} // namespace temgi
