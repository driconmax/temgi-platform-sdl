#include "temgi/Console.h"
#include "SDLPlatform.h"

SDLPlatform::SDLPlatform(temgi::Console &console) :
    console_(console), input_(console), video_()
{
}

void SDLPlatform::onConsoleStart()
{
    video_.initialize(temgi::ConsoleSpec::SCREEN_WIDTH, temgi::ConsoleSpec::SCREEN_HEIGHT, 3);
}

void SDLPlatform::onConsoleStop()
{
}

void SDLPlatform::onFrameStart()
{
}

void SDLPlatform::onFrameEnd()
{
    video_.present(console_.frameBuffer());
}
