#pragma once

#include <temgi/Console.h>
#include <SDL3/SDL.h>

class SDLInput
{
    public:
        explicit SDLInput(temgi::Console& console);

        void update();

    private:
        temgi::Console& console_;

        void processKey(SDL_Scancode key, bool pressed);
};