#include "SDLInput.h"
#include "temgi/Console.h"

SDLInput::SDLInput(temgi::Console &console) : console_(console)
{
}

void SDLInput::update()
{
    SDL_Event event;

    while(SDL_PollEvent(&event)){
        switch (event.type)
        {
            case SDL_EVENT_QUIT:
                console_.stop();
                break;

            case SDL_EVENT_KEY_DOWN:
                processKey(event.key.scancode, true);
                break;

            case SDL_EVENT_KEY_UP:
                processKey(event.key.scancode, false);
                break;
            
            default:
                break;
        }
    }
}

void SDLInput::processKey(SDL_Scancode key, bool pressed)
{
    switch(key)
    {
        case SDL_SCANCODE_Z:
            console_.setButton(
                temgi::Button::A,
                pressed
            );
            break;


        case SDL_SCANCODE_X:
            console_.setButton(
                temgi::Button::B,
                pressed
            );
            break;


        case SDL_SCANCODE_RETURN:
            console_.setButton(
                temgi::Button::Start,
                pressed
            );
            break;


        case SDL_SCANCODE_BACKSPACE:
            console_.setButton(
                temgi::Button::Select,
                pressed
            );
            break;


        case SDL_SCANCODE_UP:
            console_.setButton(
                temgi::Button::Up,
                pressed
            );
            break;


        case SDL_SCANCODE_DOWN:
            console_.setButton(
                temgi::Button::Down,
                pressed
            );
            break;


        case SDL_SCANCODE_LEFT:
            console_.setButton(
                temgi::Button::Left,
                pressed
            );
            break;


        case SDL_SCANCODE_RIGHT:
            console_.setButton(
                temgi::Button::Right,
                pressed
            );
            break;
    }
}
