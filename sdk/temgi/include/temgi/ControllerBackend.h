#pragma once

#include <temgi/Controller.h>

namespace temgi
{
    // Implemented by a platform layer (SDL, Steam Input, ...) and registered
    // with Console::setControllerBackend(). Cartridges never see this type.
    class ControllerBackend
    {
        public:
            virtual ~ControllerBackend() = default;

            virtual void rumble(int controllerIndex, float lowFrequency, float highFrequency) = 0;

            virtual bool supportsAdaptiveTriggers(int controllerIndex) const = 0;
            virtual void setTriggerEffect(int controllerIndex, Trigger trigger, const TriggerEffect& effect) = 0;
    };
} // namespace temgi
