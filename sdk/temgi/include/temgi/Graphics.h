#pragma once

#include <cstdint>

namespace temgi
{
    class GraphicsProcessor;

    class Graphics
    {
    public:
        using Pixel = std::uint32_t;

        void clear(Pixel color);

        void setPixel(
            std::uint16_t x,
            std::uint16_t y,
            Pixel color
        );

        void drawSquare(
            std::uint16_t x,
            std::uint16_t y,
            std::uint16_t width,
            std::uint16_t height,
            Pixel color
        );

        const Pixel* frameBuffer() const;

    private:
        friend class Console;

        explicit Graphics(GraphicsProcessor& processor);

        GraphicsProcessor& processor_;
    };
}