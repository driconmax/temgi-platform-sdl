#pragma once

#include <cstddef>
#include <new>
#include <utility>

namespace temgi {
    class MemoryManager;

    class Memory
    {
        public:
            void* allocateMain(std::size_t bytes, std::size_t alignment = 1);
            void* allocateFast(std::size_t bytes, std::size_t alignment = 1);

            template<typename T, typename... Args>
            T* createMain(Args&&... args)
            {
                void* address = allocateMain(
                    sizeof(T),
                    alignof(T)
                );

                if (address == nullptr)
                    return nullptr;

                return new (address) T(
                    std::forward<Args>(args)...
                );
            }

            template<typename T, typename... Args>
            T* createFast(Args&&... args)
            {
                void* address = allocateFast(
                    sizeof(T),
                    alignof(T)
                );

                if (address == nullptr)
                    return nullptr;

                return new (address) T(
                    std::forward<Args>(args)...
                );
            }

            std::size_t mainUsed() const;
            std::size_t mainAvailable() const;

            std::size_t fastUsed() const;
            std::size_t fastAvailable() const;

        private:
            friend class Console;
            friend class MemoryManager;

            explicit Memory(MemoryManager& manager);

            MemoryManager& manager_;
    };
}