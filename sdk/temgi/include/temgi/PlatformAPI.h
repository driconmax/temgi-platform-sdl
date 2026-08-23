#pragma once

#include <cstdint>

#include <temgi/Input.h>

namespace temgi
{
    class PlatformAPI
    {
        public:
            virtual ~PlatformAPI() = default;
            virtual void pollEvents() = 0;
            virtual bool isButtonPressed(Button button) const = 0;
            virtual void presentFrame(
                const std::uint32_t * pixels,
                int width,
                int height
            ) = 0;
            virtual bool shouldQuit() const = 0;
    };
} // namespace temgi
