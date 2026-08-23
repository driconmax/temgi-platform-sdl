#pragma once

#include "MemoryRegion.h"

namespace temgi {
    class MemoryManager {
        public:
            MemoryManager();
            void* allocateMain(std::size_t bytes, std::size_t alignment = 1);
            void* allocateFast(std::size_t bytes, std::size_t alignment = 1);
            std::size_t mainUsed() const;
            std::size_t fastUsed() const;

        private:
            MemoryRegion fastRam_;
            MemoryRegion mainRam_;
            MemoryRegion videoRam_;
            MemoryRegion spriteRam_;
            MemoryRegion paletteRam_;
            MemoryRegion saveRam_;
    };
}