#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <temgi/Input.h>
#include <temgi/Memory.h>
#include <temgi/Graphics.h>
#include <temgi/ConsoleEventSubscriber.h>

#include <temgi/internal/MemoryManager.h>
#include <temgi/internal/GraphicsProcessor.h>
#include <temgi/internal/CartridgeLoader.h>

namespace temgi
{
    class Console
    {
        public:
            Console();

            Input& input();
            Memory& memory();
            Graphics& graphics();

            void subscribe(ConsoleEventSubscriber& subscriber);

            bool loadCartridge(const std::string& path);
            void unloadCartridge();

            void run();
            void stop();

            void setButton(Button button, bool pressed);

            const std::uint32_t* frameBuffer() const;
            
        private:
            Input input_;
            
            MemoryManager memoryManager_;
            Memory memory_;

            GraphicsProcessor graphicsProcessor_;
            Graphics graphics_;

            CartridgeLoader cartridgeLoader_;

            bool running_ = false;
            void update(float deltaTime);

            std::vector<ConsoleEventSubscriber*> subscribers_;
    };
    
} // namespace temgi
