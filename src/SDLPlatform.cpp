#include "temgi/Console.h"
#include "SDLPlatform.h"

SDLPlatform::SDLPlatform(temgi::Console &console) :
    console_(console), input_(console), video_()
{
}

void SDLPlatform::onConsoleStart()
{
}

void SDLPlatform::onConsoleStop()
{
}

void SDLPlatform::onFrameStart()
{
}

void SDLPlatform::onFrameEnd()
{
}
