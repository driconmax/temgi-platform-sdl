#include "SDLVideo.h"
#include <iostream>

SDLVideo::SDLVideo()
{
}

SDLVideo::~SDLVideo()
{
    if(texture_ != nullptr){
        SDL_DestroyTexture(texture_);
    }
    if(renderer_ != nullptr){
        SDL_DestroyRenderer(renderer_);
    }
    if(window_ != nullptr){
        SDL_DestroyWindow(window_);
    }
}

bool SDLVideo::initialize(int width, int height, int scale)
{
    width_ = width;
    height_ = height;

    rgbaBuffer_.resize(static_cast<std::size_t>(width_) * static_cast<std::size_t>(height_));


    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr
            << "SDL_Init failed: "
            << SDL_GetError()
            << '\n';

        return false;
    }

    window_ = SDL_CreateWindow("TEMGI", width * scale, height * scale, 0);
    if (window_ == nullptr)
    {
        std::cerr
            << "SDL_CreateWindow failed: "
            << SDL_GetError()
            << '\n';

        return false;
    }

    renderer_ = SDL_CreateRenderer(window_, nullptr);
    if (renderer_ == nullptr)
    {
        std::cerr
            << "SDL_CreateRenderer failed: "
            << SDL_GetError()
            << '\n';

        return false;
    }

    texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, width_, height_);
    if (texture_ == nullptr)
    {
        std::cerr
            << "SDL_CreateTexture failed: "
            << SDL_GetError()
            << '\n';

        return false;
    }


    SDL_SetTextureScaleMode(
        texture_,
        SDL_SCALEMODE_NEAREST
    );

    return true;
}

std::uint32_t SDLVideo::convertColor(std::uint8_t color) const
{
    // 0x00 Transparent/Black
    if (color == 0x00)
    {
        return 0x000000FF;
    }

    // 0x01 - 0xD8
    // 216-color RGB cube.
    if (color <= 0xD8)
    {
        std::uint16_t index =
            static_cast<std::uint16_t>(color) - 1;

        std::uint8_t rIndex = index / 36;
        std::uint8_t gIndex = (index / 6) % 6;
        std::uint8_t bIndex = index % 6;

        constexpr std::uint8_t levels[6] =
        {
            0,
            51,
            102,
            153,
            204,
            255
        };

        std::uint8_t r = levels[rIndex];
        std::uint8_t g = levels[gIndex];
        std::uint8_t b = levels[bIndex];

        return
            (static_cast<std::uint32_t>(r) << 24) |
            (static_cast<std::uint32_t>(g) << 16) |
            (static_cast<std::uint32_t>(b) << 8)  |
            0xFF;
    }

    // 0xD9 - 0xFF
    // 39 grayscale values.

    std::uint8_t grayIndex =
        color - 0xD9;

    constexpr int grayCount = 39;

    std::uint8_t value =
        static_cast<std::uint8_t>(
            (grayIndex * 255) /
            (grayCount - 1)
        );

    return
        (static_cast<std::uint32_t>(value) << 24) |
        (static_cast<std::uint32_t>(value) << 16) |
        (static_cast<std::uint32_t>(value) << 8) |
        0xFF;
}

void SDLVideo::present(const std::uint8_t *framebuffer)
{
    const std::size_t pixelCount = static_cast<std::size_t>(width_) * static_cast<std::size_t>(height_);

    for (std::size_t i = 0; i < pixelCount; ++i)
    {
        rgbaBuffer_[i] =
            convertColor(framebuffer[i]);
    }

    SDL_UpdateTexture(texture_, nullptr, rgbaBuffer_.data(), width_ * sizeof(std::uint32_t));

    SDL_RenderClear(renderer_);

    SDL_RenderTexture(renderer_, texture_, nullptr, nullptr);

    SDL_RenderPresent(renderer_);
}
