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

        void rumble(int controllerIndex, float lowFrequency, float highFrequency) override;
        bool supportsAdaptiveTriggers(int controllerIndex) const override;
        void setTriggerEffect(int controllerIndex, temgi::Trigger trigger, const temgi::TriggerEffect& effect) override;

    private:
        temgi::Console& console_;

        static constexpr int MAX_CONTROLLERS = temgi::Input::MAX_CONTROLLERS;

        std::array<SDL_Gamepad*, MAX_CONTROLLERS> gamepads_{};
        std::array<SDL_JoystickID, MAX_CONTROLLERS> gamepadIds_{};

        void processKey(SDL_Scancode key, bool pressed);

        void handleGamepadAdded(SDL_JoystickID id);
        void handleGamepadRemoved(SDL_JoystickID id);
        void handleGamepadButton(SDL_JoystickID id, SDL_GamepadButton button, bool pressed);
        void handleGamepadAxis(SDL_JoystickID id, SDL_GamepadAxis axis, Sint16 value);

        int slotFor(SDL_JoystickID id) const;
};