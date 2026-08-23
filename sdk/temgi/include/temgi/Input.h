#pragma once
#include <array>

#include <cstddef>

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

    class Input {
        public:
            bool pressed(Button button) const;
            bool held(Button button) const;
            bool released(Button button) const;

        private:
            friend class Console;

            void setButton(Button button, bool state);
            void nextFrame();

            static constexpr std::size_t BUTTON_COUNT = static_cast<std::size_t>(Button::Count);

            std::array<bool, BUTTON_COUNT> current_{};
            std::array<bool, BUTTON_COUNT> previous_{};
    };
} // namespace temgi
