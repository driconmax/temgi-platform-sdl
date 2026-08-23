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

void SDLVideo::present(const std::uint32_t *framebuffer)
{
    SDL_UpdateTexture(texture_, nullptr, framebuffer, width_ * sizeof(std::uint32_t));

    SDL_RenderClear(renderer_);

    SDL_RenderTexture(renderer_, texture_, nullptr, nullptr);

    SDL_RenderPresent(renderer_);
}
