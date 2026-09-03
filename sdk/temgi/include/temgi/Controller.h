#pragma once

#include <array>
#include <cstddef>

namespace temgi
{
    enum class ControllerButton
    {
        South,
        East,
        West,
        North,
        Up,
        Down,
        Left,
        Right,
        L1,
        R1,
        Start,
        Select,
        LeftStick,
        RightStick,

        Count,
    };

    enum class ControllerAxis
    {
        LeftX,
        LeftY,
        RightX,
        RightY,
        LeftTrigger,
        RightTrigger,

        Count,
    };

    enum class Trigger
    {
        Left,
        Right,
    };

    enum class TriggerEffectType
    {
        None,
        Resistance,
        Vibration,
    };

    // Platform-agnostic description of a trigger feel; a backend maps this to
    // whatever the physical hardware supports (or ignores it entirely).
    struct TriggerEffect
    {
        TriggerEffectType type = TriggerEffectType::None;
        float startPosition = 0.0f;
        float strength = 0.0f;
        float frequency = 0.0f;
    };

    struct StickState
    {
        float x = 0.0f;
        float y = 0.0f;
    };

    class ControllerBackend;

    // Fluent handle returned by Controller::trigger() so cartridges can shape
    // a single trigger without needing to know about ControllerBackend.
    class TriggerHandle
    {
        public:
            TriggerHandle(ControllerBackend* backend, int controllerIndex, Trigger trigger);

            void setResistance(float startPosition, float strength);
            void setVibration(float strength, float frequency);
            void clear();

        private:
            ControllerBackend* backend_;
            int controllerIndex_;
            Trigger trigger_;

            void apply(const TriggerEffect& effect);
    };

    class Controller
    {
        public:
            bool isConnected() const;

            bool isPressed(ControllerButton button) const;
            bool isHeld(ControllerButton button) const;
            bool isReleased(ControllerButton button) const;

            StickState leftStick() const;
            StickState rightStick() const;

            float leftTrigger() const;
            float rightTrigger() const;

            void rumble(float lowFrequency, float highFrequency);

            bool supportsAdaptiveTriggers() const;
            TriggerHandle trigger(Trigger which);

        private:
            friend class Console;
            friend class Input;

            void setConnected(bool connected);
            void setButton(ControllerButton button, bool pressed);
            void setAxis(ControllerAxis axis, float value);
            void nextFrame();

            void bindBackend(ControllerBackend* backend, int index);

            static constexpr std::size_t BUTTON_COUNT = static_cast<std::size_t>(ControllerButton::Count);

            bool connected_ = false;

            std::array<bool, BUTTON_COUNT> current_{};
            std::array<bool, BUTTON_COUNT> previous_{};

            float leftX_ = 0.0f;
            float leftY_ = 0.0f;
            float rightX_ = 0.0f;
            float rightY_ = 0.0f;
            float leftTrigger_ = 0.0f;
            float rightTrigger_ = 0.0f;

            ControllerBackend* backend_ = nullptr;
            int index_ = -1;
    };
} // namespace temgi
