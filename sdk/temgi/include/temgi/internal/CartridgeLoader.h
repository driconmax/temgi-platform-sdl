#pragma once

#include <string>
#include <vector>

#include <temgi/CartridgeAPI.h>
#include <temgi/Asset.h>

namespace temgi
{
    class CartridgeLoader {
        public: 
            CartridgeLoader();
            ~CartridgeLoader();

            bool load(const std::string& path);
            void unload();

            CartridgeAPI* cartridge();

            Asset asset(const std::string& name) const;

        private:
            std::vector<std::uint8_t> cartridgeData_;
            std::uint32_t assetCount_ = 0;
            std::uint32_t assetTableOffset_ = 0;
            std::uint32_t assetDataOffset_ = 0;

            void* libraryHandle_ = nullptr;
            CartridgeAPI* cartridge_ = nullptr;

            std::string extractedLibraryPath_;

    };
} // namespace temgi
