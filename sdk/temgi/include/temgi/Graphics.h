#pragma once

#include <cstdint>
#include <string>

#include <temgi/Image.h>
#include <temgi/Animation.h>
#include <temgi/Font.h>

namespace temgi
{
    class GraphicsProcessor;

    class Graphics
    {
    public:
        using Pixel = std::uint8_t;

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

        void drawImage(
            const Image& image,
            std::uint16_t x,
            std::uint16_t y
        );

        void drawAnimationFrame(
            const Animation& animation,
            std::uint16_t frame,
            std::uint16_t x,
            std::uint16_t y
        );

        void drawText(
            const std::string& text,
            std::uint16_t x,
            std::uint16_t y,
            Pixel color
        );

        void drawText(
            const std::string& text,
            std::uint16_t x,
            std::uint16_t y,
            Pixel color,
            const Font& font
        );

        const Pixel* frameBuffer() const;

    private:
        friend class Console;

        explicit Graphics(GraphicsProcessor& processor);

        GraphicsProcessor& processor_;
    };
}