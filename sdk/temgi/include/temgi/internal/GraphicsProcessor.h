#pragma once

#include <array>
#include <cstdint>

#include <temgi/ConsoleSpec.h>
#include <temgi/Image.h>

namespace temgi
{
    class GraphicsProcessor
    {
        
        public:
            using Pixel = std::uint8_t;
            
            void clear(Pixel color);
            void setPixel(std::uint16_t x, std::uint16_t y, Pixel color);
            void drawSquare(std::uint16_t x, std::uint16_t y, std::uint16_t size_x, std::uint16_t size_y, Pixel color);

            void drawImage(
                const Image& image,
                std::uint16_t x,
                std::uint16_t y
            );

            const Pixel* framebuffer() const;

        private:
            std::array<Pixel, ConsoleSpec::SCREEN_WIDTH * ConsoleSpec::SCREEN_HEIGHT> framebuffer_;
    };

} // namespace temgi
