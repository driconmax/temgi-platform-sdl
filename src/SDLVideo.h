#pragma once

#include <cstdint>
#include <SDL3/SDL.h>

class SDLVideo {
    public:
        SDLVideo();
        ~SDLVideo();

        bool initialize(
            int width,
            int height,
            int scale = 3
        );

        void present(const std::uint8_t* framebuffer);

    private:
        SDL_Window* window_ = nullptr;
        SDL_Renderer* renderer_ = nullptr;
        SDL_Texture* texture_ = nullptr;

        int width_ = 0;
        int height_ = 0;

        std::vector<std::uint32_t> rgbaBuffer_;
        std::uint32_t convertColor(std::uint8_t color) const;
};