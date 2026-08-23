#pragma once

#include <cstdint>
#include <SDL3/SDL.h>

class SDLVideo {
    public:
        SDLVideo();
        ~SDLVideo();

        bool initialize();

        void present(const std::uint32_t* framebuffer);

    private:
        SDL_Window* window_ = nullptr;
        SDL_Renderer* renderer_ = nullptr;
        SDL_Texture* texture_ = nullptr;
};