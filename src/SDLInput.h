#pragma once

#include <array>

#include <temgi/Console.h>
#include <temgi/ControllerBackend.h>
#include <SDL3/SDL.h>

// Default controller backend: plain SDL gamepad support (works without Steam).
// Adaptive triggers are a no-op here; a future SteamInputBackend can override
// them for DualSense without the cartridge/Console ever knowing the difference.
class SDLInput : public temgi::ControllerBackend
{
    public:
        explicit SDLInput(temgi::Console& console);
        ~SDLInput() override;

        void initialize();
        void update();

        // Forces every connected controller's rumble/trigger effects off. Call
        // on game start/stop so nothing is left buzzing/resisting on the
        // physical hardware once TEMGI isn't driving it anymore.
        void clearAllEffects();

        void rumble(int controllerIndex, float lowFrequency, float highFrequency) override;
        bool supportsAdaptiveTriggers(int controllerIndex) const override;
        void setTriggerEffect(int controllerIndex, temgi::Trigger trigger, const temgi::TriggerEffect& effect) override;

    private:
        temgi::Console& console_;

        static constexpr int MAX_CONTROLLERS = temgi::Input::MAX_CONTROLLERS;

        std::array<SDL_Gamepad*, MAX_CONTROLLERS> gamepads_{};
        std::array<SDL_JoystickID, MAX_CONTROLLERS> gamepadIds_{};

        // Slot 0 also drives the legacy keyboard-style Button enum, since existing
        // game code only reads console.input().pressed(Button::...), not Controller.
        bool legacySouthHeld_ = false;
        bool legacyRightTriggerHeld_ = false;
        void updateLegacyDrillButton();

        void processKey(SDL_Scancode key, bool pressed);

        void handleGamepadAdded(SDL_JoystickID id);
        void handleGamepadRemoved(SDL_JoystickID id);
        void handleGamepadButton(SDL_JoystickID id, SDL_GamepadButton button, bool pressed);
        void handleGamepadAxis(SDL_JoystickID id, SDL_GamepadAxis axis, Sint16 value);

        int slotFor(SDL_JoystickID id) const;

        // DualSense sends rumble and trigger-effect state in ONE combined output
        // report; SDL_RumbleGamepad and our raw SDL_SendGamepadEffect calls would
        // otherwise each clobber the other's bytes back to zero. So for PS5
        // gamepads we track the full desired state ourselves and always resend
        // it as a whole (see sendEffects()).
        struct GamepadEffects
        {
            Uint8 rumbleLow = 0;
            Uint8 rumbleHigh = 0;
            Uint64 rumbleExpireTicks = 0; // SDL_GetTicks() deadline; 0 = not decaying

            Uint8 rightMode = 0;
            Uint8 rightPos = 0;
            Uint8 rightAmp = 0;
            Uint8 rightFreq = 0;

            Uint8 leftMode = 0;
            Uint8 leftPos = 0;
            Uint8 leftAmp = 0;
            Uint8 leftFreq = 0;
        };

        std::array<GamepadEffects, MAX_CONTROLLERS> effects_{};

        void sendEffects(int controllerIndex);
        void decayRumble();
};