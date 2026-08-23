#pragma once

#include <temgi/Console.h>

class SDLInput
{
    public:
        explicit SDLInput(temgi::Console& console);

        void update();

    private:
        temgi::Console& console_;
};