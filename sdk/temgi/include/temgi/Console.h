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

            Asset asset(const std::string& name) const;

            void run();
            void stop();

            void setButton(Button button, bool pressed);

            void fatalError(const std::string& message);

            const std::uint8_t* frameBuffer() const;
            
        private:
            Input input_;
            
            MemoryManager memoryManager_;
            Memory memory_;

            GraphicsProcessor graphicsProcessor_;
            Graphics graphics_;

            CartridgeLoader cartridgeLoader_;

            bool failed_ = false;
            bool running_ = false;
            void update(float deltaTime);

            std::string errorMessage_;
            void drawErrorOverlay();

            std::vector<ConsoleEventSubscriber*> subscribers_;
    };
    
} // namespace temgi
