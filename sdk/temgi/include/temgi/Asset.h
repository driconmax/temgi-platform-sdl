#pragma once

#include <cstddef>
#include <cstdint>

namespace temgi
{
    struct Asset {
        const std::uint8_t* data = nullptr;
        std::size_t size = 0;

        bool valid() const {
            return data != nullptr;
        }
    };
} // namespace temgi
