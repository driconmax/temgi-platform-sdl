#pragma once

#include <temgi/Console.h>

#include "SDLInput.h"
#include "SDLVideo.h"

class SDLPlatform : public temgi::ConsoleEventSubscriber {
    public:
        explicit SDLPlatform (temgi::Console& console);

        void onConsoleStart() override;
        void onConsoleStop() override;
        void onFrameStart() override;
        void onFrameEnd() override;

    private:
        temgi::Console& console_;
        SDLInput input_;
        SDLVideo video_;
};