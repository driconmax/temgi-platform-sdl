#pragma once

#include <string>

#include <temgi/CartridgeAPI.h>

namespace temgi
{
    class CartridgeLoader {
        public: 
            CartridgeLoader();
            ~CartridgeLoader();

            bool load(const std::string& path);
            void unload();

            CartridgeAPI* cartridge();

        private:
            void* libraryHandle_ = nullptr;
            CartridgeAPI* cartridge_ = nullptr;

            std::string extractedLibraryPath_;

    };
} // namespace temgi
