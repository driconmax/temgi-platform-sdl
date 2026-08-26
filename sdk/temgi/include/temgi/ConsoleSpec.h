#pragma once

#include <cstddef>

namespace temgi {
	struct ConsoleSpec {
		static constexpr std::size_t FAST_RAM_SIZE			= 32 * 1024;
		static constexpr std::size_t MAIN_RAM_SIZE			= 8 * 1024 * 1024;
		static constexpr std::size_t VIDEO_RAM_SIZE			= 96 * 1024;
		static constexpr std::size_t SPRITE_RAM_SIZE		= 1 * 1024;
		static constexpr std::size_t PALETTE_RAM_SIZE		= 1 * 1024;
		static constexpr std::size_t SAVE_RAM_SIZE			= 1 * 1024;

		static constexpr auto FRAME_DURATION 				= std::chrono::microseconds(16666);

		static constexpr std::size_t MAX_CARTRIDGE_SIZE		= 32 * 1024 * 1024;

		static constexpr int SCREEN_WIDTH		= 256;
		static constexpr int SCREEN_HEIGHT		= 240;
	};
}