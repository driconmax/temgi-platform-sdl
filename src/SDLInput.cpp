#include "SDLInput.h"
#include "temgi/Console.h"

#include <algorithm>

namespace
{
    bool mapButton(SDL_GamepadButton button, temgi::ControllerButton& out)
    {
        switch (button)
        {
            case SDL_GAMEPAD_BUTTON_SOUTH: out = temgi::ControllerButton::South; return true;
            case SDL_GAMEPAD_BUTTON_EAST: out = temgi::ControllerButton::East; return true;
            case SDL_GAMEPAD_BUTTON_WEST: out = temgi::ControllerButton::West; return true;
            case SDL_GAMEPAD_BUTTON_NORTH: out = temgi::ControllerButton::North; return true;
            case SDL_GAMEPAD_BUTTON_DPAD_UP: out = temgi::ControllerButton::Up; return true;
            case SDL_GAMEPAD_BUTTON_DPAD_DOWN: out = temgi::ControllerButton::Down; return true;
            case SDL_GAMEPAD_BUTTON_DPAD_LEFT: out = temgi::ControllerButton::Left; return true;
            case SDL_GAMEPAD_BUTTON_DPAD_RIGHT: out = temgi::ControllerButton::Right; return true;
            case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER: out = temgi::ControllerButton::L1; return true;
            case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER: out = temgi::ControllerButton::R1; return true;
            case SDL_GAMEPAD_BUTTON_START: out = temgi::ControllerButton::Start; return true;
            case SDL_GAMEPAD_BUTTON_BACK: out = temgi::ControllerButton::Select; return true;
            case SDL_GAMEPAD_BUTTON_LEFT_STICK: out = temgi::ControllerButton::LeftStick; return true;
            case SDL_GAMEPAD_BUTTON_RIGHT_STICK: out = temgi::ControllerButton::RightStick; return true;
            default: return false;
        }
    }

    float normalizeStickAxis(Sint16 value)
    {
        constexpr float MAX = 32768.0f;
        return std::clamp(static_cast<float>(value) / MAX, -1.0f, 1.0f);
    }

    float normalizeTriggerAxis(Sint16 value)
    {
        constexpr float MAX = 32767.0f;
        return std::clamp(static_cast<float>(value) / MAX, 0.0f, 1.0f);
    }

    constexpr float TRIGGER_PRESS_THRESHOLD = 0.35f;

    // Raw DualSense trigger-effect output report. SDL_RumbleGamepadTriggers is
    // a hard-coded no-op for PS5 in SDL itself, so this bypasses it entirely via
    // SDL_SendGamepadEffect. Byte layout (DS5EffectsState_t) confirmed from SDL's
    // own hidapi PS5 driver source; the enable bits / mode byte are reverse-engineered
    // (not documented by Sony or SDL) and verified live against real hardware.
    constexpr Uint8 TRIGGER_ENABLE_RIGHT = 0x04;
    constexpr Uint8 TRIGGER_ENABLE_LEFT = 0x08;
    constexpr int TRIGGER_REPORT_OFFSET_RIGHT = 10;
    constexpr int TRIGGER_REPORT_OFFSET_LEFT = 21;
    constexpr Uint8 TRIGGER_MODE_OFF = 0x00;
    constexpr Uint8 TRIGGER_MODE_VIBRATION = 0x06;
    constexpr Uint8 TRIGGER_EFFECT_POSITION = 3;

    // Existing game code only reads the legacy Button enum; South/Start/Select/
    // Dpad/right shoulder map straight across (right trigger is handled separately
    // since it also doubles as the mining button).
    bool mapLegacyButton(SDL_GamepadButton button, temgi::Button& out)
    {
        switch (button)
        {
            case SDL_GAMEPAD_BUTTON_EAST: out = temgi::Button::B; return true;
            case SDL_GAMEPAD_BUTTON_START: out = temgi::Button::Start; return true;
            case SDL_GAMEPAD_BUTTON_BACK: out = temgi::Button::Select; return true;
            case SDL_GAMEPAD_BUTTON_DPAD_UP: out = temgi::Button::Up; return true;
            case SDL_GAMEPAD_BUTTON_DPAD_DOWN: out = temgi::Button::Down; return true;
            case SDL_GAMEPAD_BUTTON_DPAD_LEFT: out = temgi::Button::Left; return true;
            case SDL_GAMEPAD_BUTTON_DPAD_RIGHT: out = temgi::Button::Right; return true;
            case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER: out = temgi::Button::R1; return true;
            default: return false;
        }
    }
}

SDLInput::SDLInput(temgi::Console &console) : console_(console)
{
}

SDLInput::~SDLInput()
{
    for (SDL_Gamepad* gamepad : gamepads_)
    {
        if (gamepad != nullptr)
        {
            SDL_CloseGamepad(gamepad);
        }
    }
}

void SDLInput::initialize()
{
    SDL_InitSubSystem(SDL_INIT_GAMEPAD);

    console_.setControllerBackend(this);
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

            case SDL_EVENT_GAMEPAD_ADDED:
                handleGamepadAdded(event.gdevice.which);
                break;

            case SDL_EVENT_GAMEPAD_REMOVED:
                handleGamepadRemoved(event.gdevice.which);
                break;

            case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
                handleGamepadButton(event.gbutton.which, static_cast<SDL_GamepadButton>(event.gbutton.button), true);
                break;

            case SDL_EVENT_GAMEPAD_BUTTON_UP:
                handleGamepadButton(event.gbutton.which, static_cast<SDL_GamepadButton>(event.gbutton.button), false);
                break;

            case SDL_EVENT_GAMEPAD_AXIS_MOTION:
                handleGamepadAxis(event.gaxis.which, static_cast<SDL_GamepadAxis>(event.gaxis.axis), event.gaxis.value);
                break;

            default:
                break;
        }
    }
}

void SDLInput::rumble(int controllerIndex, float lowFrequency, float highFrequency)
{
    if (controllerIndex < 0 || controllerIndex >= MAX_CONTROLLERS) return;

    SDL_Gamepad* gamepad = gamepads_[static_cast<std::size_t>(controllerIndex)];
    if (gamepad == nullptr) return;

    constexpr Uint32 RUMBLE_DURATION_MS = 200;

    SDL_RumbleGamepad(
        gamepad,
        static_cast<Uint16>(std::clamp(lowFrequency, 0.0f, 1.0f) * 0xFFFF),
        static_cast<Uint16>(std::clamp(highFrequency, 0.0f, 1.0f) * 0xFFFF),
        RUMBLE_DURATION_MS
    );
}

bool SDLInput::supportsAdaptiveTriggers(int controllerIndex) const
{
    if (controllerIndex < 0 || controllerIndex >= MAX_CONTROLLERS) return false;

    SDL_Gamepad* gamepad = gamepads_[static_cast<std::size_t>(controllerIndex)];
    if (gamepad == nullptr) return false;

    // The raw report below only targets the DualSense protocol.
    return SDL_GetGamepadType(gamepad) == SDL_GAMEPAD_TYPE_PS5;
}

void SDLInput::setTriggerEffect(int controllerIndex, temgi::Trigger trigger, const temgi::TriggerEffect& effect)
{
    if (controllerIndex < 0 || controllerIndex >= MAX_CONTROLLERS) return;

    SDL_Gamepad* gamepad = gamepads_[static_cast<std::size_t>(controllerIndex)];
    if (gamepad == nullptr) return;

    Uint8 data[47] = {0};

    const bool isRight = (trigger == temgi::Trigger::Right);
    const int offset = isRight ? TRIGGER_REPORT_OFFSET_RIGHT : TRIGGER_REPORT_OFFSET_LEFT;

    data[0] = isRight ? TRIGGER_ENABLE_RIGHT : TRIGGER_ENABLE_LEFT;

    if (effect.type == temgi::TriggerEffectType::Vibration)
    {
        data[offset] = TRIGGER_MODE_VIBRATION;
        data[offset + 1] = TRIGGER_EFFECT_POSITION;
        data[offset + 2] = static_cast<Uint8>(std::clamp(effect.strength, 0.0f, 1.0f) * 255.0f);
        data[offset + 3] = static_cast<Uint8>(std::clamp(effect.frequency, 0.0f, 255.0f));
    }
    else
    {
        data[offset] = TRIGGER_MODE_OFF;
    }

    SDL_SendGamepadEffect(gamepad, data, sizeof(data));
}

void SDLInput::handleGamepadAdded(SDL_JoystickID id)
{
    for (int i = 0; i < MAX_CONTROLLERS; ++i)
    {
        if (gamepads_[static_cast<std::size_t>(i)] == nullptr)
        {
            SDL_Gamepad* gamepad = SDL_OpenGamepad(id);
            if (gamepad == nullptr) return;

            gamepads_[static_cast<std::size_t>(i)] = gamepad;
            gamepadIds_[static_cast<std::size_t>(i)] = id;
            console_.setControllerConnected(i, true);
            return;
        }
    }
}

void SDLInput::handleGamepadRemoved(SDL_JoystickID id)
{
    int slot = slotFor(id);
    if (slot < 0) return;

    SDL_CloseGamepad(gamepads_[static_cast<std::size_t>(slot)]);
    gamepads_[static_cast<std::size_t>(slot)] = nullptr;
    gamepadIds_[static_cast<std::size_t>(slot)] = 0;
    console_.setControllerConnected(slot, false);

    if (slot == 0)
    {
        legacySouthHeld_ = false;
        legacyRightTriggerHeld_ = false;
        updateLegacyDrillButton();
    }
}

void SDLInput::handleGamepadButton(SDL_JoystickID id, SDL_GamepadButton button, bool pressed)
{
    int slot = slotFor(id);
    if (slot < 0) return;

    temgi::ControllerButton mapped;
    if (mapButton(button, mapped))
    {
        console_.setControllerButton(slot, mapped, pressed);
    }

    if (slot != 0) return;

    if (button == SDL_GAMEPAD_BUTTON_SOUTH)
    {
        legacySouthHeld_ = pressed;
        updateLegacyDrillButton();
        return;
    }

    temgi::Button legacy;
    if (mapLegacyButton(button, legacy))
    {
        console_.setButton(legacy, pressed);
    }
}

void SDLInput::updateLegacyDrillButton()
{
    console_.setButton(temgi::Button::A, legacySouthHeld_ || legacyRightTriggerHeld_);
}

void SDLInput::handleGamepadAxis(SDL_JoystickID id, SDL_GamepadAxis axis, Sint16 value)
{
    int slot = slotFor(id);
    if (slot < 0) return;

    switch (axis)
    {
        case SDL_GAMEPAD_AXIS_LEFTX:
            console_.setControllerAxis(slot, temgi::ControllerAxis::LeftX, normalizeStickAxis(value));
            break;

        case SDL_GAMEPAD_AXIS_LEFTY:
            console_.setControllerAxis(slot, temgi::ControllerAxis::LeftY, normalizeStickAxis(value));
            break;

        case SDL_GAMEPAD_AXIS_RIGHTX:
            console_.setControllerAxis(slot, temgi::ControllerAxis::RightX, normalizeStickAxis(value));
            break;

        case SDL_GAMEPAD_AXIS_RIGHTY:
            console_.setControllerAxis(slot, temgi::ControllerAxis::RightY, normalizeStickAxis(value));
            break;

        case SDL_GAMEPAD_AXIS_LEFT_TRIGGER:
            console_.setControllerAxis(slot, temgi::ControllerAxis::LeftTrigger, normalizeTriggerAxis(value));
            break;

        case SDL_GAMEPAD_AXIS_RIGHT_TRIGGER:
        {
            const float normalized = normalizeTriggerAxis(value);
            console_.setControllerAxis(slot, temgi::ControllerAxis::RightTrigger, normalized);

            // Right trigger doubles as the mining button (Button::A) for slot 0.
            if (slot == 0)
            {
                const bool held = normalized > TRIGGER_PRESS_THRESHOLD;
                if (held != legacyRightTriggerHeld_)
                {
                    legacyRightTriggerHeld_ = held;
                    updateLegacyDrillButton();
                }
            }
            break;
        }

        default:
            break;
    }
}

int SDLInput::slotFor(SDL_JoystickID id) const
{
    for (int i = 0; i < MAX_CONTROLLERS; ++i)
    {
        if (gamepads_[static_cast<std::size_t>(i)] != nullptr && gamepadIds_[static_cast<std::size_t>(i)] == id)
        {
            return i;
        }
    }
    return -1;
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
