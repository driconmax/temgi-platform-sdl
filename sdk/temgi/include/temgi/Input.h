#pragma once
#include <array>

#include <cstddef>

#include <temgi/Controller.h>

namespace temgi
{
    enum class Button {
        Up,
        Down,
        Left,
        Right,
        A,
        B,
        Start,
        Select,
        R1,
        R2,

        Count,
    };

    class ControllerBackend;

    class Input {
        public:
            Input();

            static constexpr int MAX_CONTROLLERS = 4;

            bool pressed(Button button) const;
            bool held(Button button) const;
            bool released(Button button) const;

            // Player index 0..MAX_CONTROLLERS-1; joysticks/gamepads live here,
            // independent of the keyboard-driven digital buttons above.
            Controller& controller(int index);
            const Controller& controller(int index) const;

        private:
            friend class Console;

            void setButton(Button button, bool state);
            void nextFrame();

            void setControllerBackend(ControllerBackend* backend);

            static constexpr std::size_t BUTTON_COUNT = static_cast<std::size_t>(Button::Count);

            std::array<bool, BUTTON_COUNT> current_{};
            std::array<bool, BUTTON_COUNT> previous_{};

            std::array<Controller, MAX_CONTROLLERS> controllers_{};
    };
} // namespace temgi
