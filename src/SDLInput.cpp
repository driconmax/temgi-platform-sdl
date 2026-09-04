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

    // Mode 0x06 ("Vibration") turned out to have its OWN fixed internal buzz
    // rate baked into the firmware - toggling it on/off with our own timer just
    // flickers a fixed-rate buzz, it doesn't change its speed (verified by hand
    // across every documented mode: they all felt identical regardless of the
    // "frequency" byte). Mode 0x01 ("Feedback") is a static, non-oscillating
    // constant resistance with no internal timing at all - so toggling THAT on
    // and off makes our own software timer the only source of any pulsing,
    // giving us genuine control over the perceived speed.
    constexpr Uint8 TRIGGER_MODE_OFF = 0x00; // per pydualsense's TriggerModes.Off - explicit release, not just "don't touch"
    constexpr Uint8 TRIGGER_MODE_FEEDBACK = 0x01;
    constexpr Uint8 TRIGGER_EFFECT_POSITION = 3;

    // A fast on/off toggle of a static resistance isn't felt at all - your
    // finger needs the resistance held for a bit to register it as a push.
    // So instead of a symmetric buzz, we pulse it as short, fixed-duration
    // "knocks" (long enough to actually feel) separated by a gap whose length
    // tracks the game's requested frequency - the gap shrinking is what makes
    // the knock rate speed up with drillFps.
    constexpr Uint64 KNOCK_ON_MS = 70;
    constexpr float KNOCK_RATE_INPUT_MAX = 60.0f; // roughly the game's max drillFps
    constexpr float KNOCK_RATE_MIN = 1.5f; // knocks/sec at the slowest drill speed
    constexpr float KNOCK_RATE_MAX = 8.0f; // knocks/sec at the fastest drill speed

    float scaleKnockPeriodMs(float frequency)
    {
        const float t = std::clamp(frequency, 0.0f, KNOCK_RATE_INPUT_MAX) / KNOCK_RATE_INPUT_MAX;
        const float knocksPerSecond = KNOCK_RATE_MIN + t * (KNOCK_RATE_MAX - KNOCK_RATE_MIN);
        return 1000.0f / knocksPerSecond;
    }

    Uint8 toByteValue(float normalized)
    {
        return static_cast<Uint8>(std::clamp(normalized, 0.0f, 1.0f) * 255.0f);
    }

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
    clearAllEffects();

    for (SDL_Gamepad* gamepad : gamepads_)
    {
        if (gamepad != nullptr)
        {
            SDL_CloseGamepad(gamepad);
        }
    }
}

void SDLInput::clearAllEffects()
{
    for (int i = 0; i < MAX_CONTROLLERS; ++i)
    {
        if (gamepads_[static_cast<std::size_t>(i)] == nullptr) continue;

        GamepadEffects& state = effects_[static_cast<std::size_t>(i)];

        // Force an explicit release first: an unset trigger-enable bit means
        // "don't touch", not "off", so simply zeroing our state and resending
        // would leave a mid-flight resistance stuck on the hardware.
        state.rumbleLow = 0;
        state.rumbleHigh = 0;
        state.rumbleExpireTicks = 0;
        state.rightPulseOn = false;
        state.leftPulseOn = false;
        sendEffects(i);

        effects_[static_cast<std::size_t>(i)] = GamepadEffects{};
        sendEffects(i);
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

    decayRumble();
    updateTriggerPulses();
}

void SDLInput::rumble(int controllerIndex, float lowFrequency, float highFrequency)
{
    if (controllerIndex < 0 || controllerIndex >= MAX_CONTROLLERS) return;

    SDL_Gamepad* gamepad = gamepads_[static_cast<std::size_t>(controllerIndex)];
    if (gamepad == nullptr) return;

    // Non-DualSense pads have no trigger-effect report to race with, so SDL's
    // own timed rumble is fine as-is.
    if (SDL_GetGamepadType(gamepad) != SDL_GAMEPAD_TYPE_PS5)
    {
        constexpr Uint32 RUMBLE_DURATION_MS = 200;
        SDL_RumbleGamepad(
            gamepad,
            static_cast<Uint16>(std::clamp(lowFrequency, 0.0f, 1.0f) * 0xFFFF),
            static_cast<Uint16>(std::clamp(highFrequency, 0.0f, 1.0f) * 0xFFFF),
            RUMBLE_DURATION_MS
        );
        return;
    }

    // DualSense: rumble and trigger effects share one output report, so track
    // both and always resend the combined state (see sendEffects()) instead of
    // letting SDL's own rumble call clobber our trigger bytes back to zero.
    constexpr Uint32 RUMBLE_HOLD_MS = 200;

    GamepadEffects& state = effects_[static_cast<std::size_t>(controllerIndex)];
    state.rumbleLow = toByteValue(lowFrequency);
    state.rumbleHigh = toByteValue(highFrequency);
    state.rumbleExpireTicks = (state.rumbleLow != 0 || state.rumbleHigh != 0) ? (SDL_GetTicks() + RUMBLE_HOLD_MS) : 0;

    sendEffects(controllerIndex);
}

void SDLInput::decayRumble()
{
    const Uint64 now = SDL_GetTicks();

    for (int i = 0; i < MAX_CONTROLLERS; ++i)
    {
        GamepadEffects& state = effects_[static_cast<std::size_t>(i)];
        if (state.rumbleExpireTicks == 0 || now < state.rumbleExpireTicks) continue;

        state.rumbleLow = 0;
        state.rumbleHigh = 0;
        state.rumbleExpireTicks = 0;
        sendEffects(i);
    }
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
    if (SDL_GetGamepadType(gamepad) != SDL_GAMEPAD_TYPE_PS5) return;

    const bool isRight = (trigger == temgi::Trigger::Right);
    GamepadEffects& state = effects_[static_cast<std::size_t>(controllerIndex)];

    bool& active = isRight ? state.rightActive : state.leftActive;
    Uint8& amp = isRight ? state.rightAmp : state.leftAmp;
    float& knockPeriodMs = isRight ? state.rightKnockPeriodMs : state.leftKnockPeriodMs;
    bool& pulseOn = isRight ? state.rightPulseOn : state.leftPulseOn;
    Uint64& nextToggle = isRight ? state.rightNextToggleTicks : state.leftNextToggleTicks;

    if (effect.type == temgi::TriggerEffectType::Vibration)
    {
        const bool wasActive = active;
        active = true;
        amp = toByteValue(effect.strength);
        knockPeriodMs = scaleKnockPeriodMs(effect.frequency);

        if (!wasActive)
        {
            // Just started: kick off with a knock immediately rather than
            // waiting for the first gap to elapse.
            pulseOn = true;
            nextToggle = SDL_GetTicks() + KNOCK_ON_MS;
            sendEffects(controllerIndex);
        }
        // else: already pulsing - updateTriggerPulses() keeps driving it at
        // the (possibly just-updated) rate; no need to resend here.
    }
    else if (active)
    {
        // Force one explicit release (enable bit still set, mode=Off) before
        // dropping active - an unset enable bit means "don't touch", not
        // "off", so clearing active first would leave it stuck resisting.
        pulseOn = false;
        sendEffects(controllerIndex);
        active = false;
        nextToggle = 0;
    }
}

void SDLInput::updateTriggerPulses()
{
    const Uint64 now = SDL_GetTicks();

    for (int i = 0; i < MAX_CONTROLLERS; ++i)
    {
        GamepadEffects& state = effects_[static_cast<std::size_t>(i)];
        bool changed = false;

        if (state.rightActive && now >= state.rightNextToggleTicks)
        {
            state.rightPulseOn = !state.rightPulseOn;
            const Uint64 period = static_cast<Uint64>(state.rightKnockPeriodMs);
            state.rightNextToggleTicks = now + (state.rightPulseOn ? KNOCK_ON_MS : (period > KNOCK_ON_MS ? period - KNOCK_ON_MS : 1));
            changed = true;
        }

        if (state.leftActive && now >= state.leftNextToggleTicks)
        {
            state.leftPulseOn = !state.leftPulseOn;
            const Uint64 period = static_cast<Uint64>(state.leftKnockPeriodMs);
            state.leftNextToggleTicks = now + (state.leftPulseOn ? KNOCK_ON_MS : (period > KNOCK_ON_MS ? period - KNOCK_ON_MS : 1));
            changed = true;
        }

        if (changed) sendEffects(i);
    }
}

void SDLInput::sendEffects(int controllerIndex)
{
    if (controllerIndex < 0 || controllerIndex >= MAX_CONTROLLERS) return;

    SDL_Gamepad* gamepad = gamepads_[static_cast<std::size_t>(controllerIndex)];
    if (gamepad == nullptr) return;

    const GamepadEffects& state = effects_[static_cast<std::size_t>(controllerIndex)];

    Uint8 data[47] = {0};
    Uint8 enableBits = 0;

    if (state.rumbleLow != 0 || state.rumbleHigh != 0)
    {
        enableBits |= 0x01; // classic rumble emulation
        enableBits |= 0x02; // disable audio haptics
        data[2] = state.rumbleHigh; // ucRumbleRight
        data[3] = state.rumbleLow;  // ucRumbleLeft
    }

    // Verified against pydualsense (a working open-source DualSense library):
    // the enable bit means "apply the mode/force bytes below", not "trigger is
    // currently active" - so it must stay set for the whole time we're
    // managing this trigger, or the firmware just ignores the packet and
    // keeps whatever resistance was last applied. The actual on/off toggling
    // has to happen via the MODE byte itself (Feedback vs Off), not the
    // enable bit - that's the bug that made every knock feel like one
    // constant, never-releasing resistance instead of a pulse.
    if (state.rightActive) enableBits |= TRIGGER_ENABLE_RIGHT;
    if (state.leftActive) enableBits |= TRIGGER_ENABLE_LEFT;

    data[0] = enableBits;

    if (state.rightActive)
    {
        if (state.rightPulseOn)
        {
            data[TRIGGER_REPORT_OFFSET_RIGHT + 0] = TRIGGER_MODE_FEEDBACK;
            data[TRIGGER_REPORT_OFFSET_RIGHT + 1] = TRIGGER_EFFECT_POSITION;
            data[TRIGGER_REPORT_OFFSET_RIGHT + 2] = state.rightAmp;
        }
        else
        {
            data[TRIGGER_REPORT_OFFSET_RIGHT + 0] = TRIGGER_MODE_OFF;
        }
    }

    if (state.leftActive)
    {
        if (state.leftPulseOn)
        {
            data[TRIGGER_REPORT_OFFSET_LEFT + 0] = TRIGGER_MODE_FEEDBACK;
            data[TRIGGER_REPORT_OFFSET_LEFT + 1] = TRIGGER_EFFECT_POSITION;
            data[TRIGGER_REPORT_OFFSET_LEFT + 2] = state.leftAmp;
        }
        else
        {
            data[TRIGGER_REPORT_OFFSET_LEFT + 0] = TRIGGER_MODE_OFF;
        }
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
            effects_[static_cast<std::size_t>(i)] = GamepadEffects{};
            sendEffects(i); // known-clean slate in case hardware had leftover state (e.g. previous crashed run)
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
