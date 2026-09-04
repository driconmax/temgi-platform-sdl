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
        //
        // The firmware's own trigger-effect "frequency" byte does NOT control
        // oscillation speed, and self-oscillating modes (e.g. "Vibration") have
        // their own fixed internal buzz rate that dominates regardless of how we
        // toggle them - both verified by hand. So we drive a static, non-
        // oscillating resistance ourselves as a series of short "knocks": a
        // fixed-duration on-pulse (long enough to actually register as felt
        // resistance) separated by a gap whose length is computed from the
        // game's requested frequency (see updateTriggerPulses()).
        struct GamepadEffects
        {
            Uint8 rumbleLow = 0;
            Uint8 rumbleHigh = 0;
            Uint64 rumbleExpireTicks = 0; // SDL_GetTicks() deadline; 0 = not decaying

            bool rightActive = false;
            Uint8 rightAmp = 0;
            float rightKnockPeriodMs = 0.0f; // full on+off cycle length
            Uint64 rightNextToggleTicks = 0;
            bool rightPulseOn = false;

            bool leftActive = false;
            Uint8 leftAmp = 0;
            float leftKnockPeriodMs = 0.0f;
            Uint64 leftNextToggleTicks = 0;
            bool leftPulseOn = false;
        };

        std::array<GamepadEffects, MAX_CONTROLLERS> effects_{};

        void sendEffects(int controllerIndex);
        void decayRumble();
        void updateTriggerPulses();
};