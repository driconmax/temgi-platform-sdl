cmake_minimum_required(VERSION 3.20)

project(temgi-platform-sdl LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL3 REQUIRED)
find_package(temgi-console REQUIRED)

find_package(SDL3 REQUIRED CONFIG)

add_executable(temgi-sdl
    src/main.cpp
    src/SDLPlatform.cpp
    src/SDLVideo.cpp
    src/SDLInput.cpp
    src/SDLAudio.cpp
)

target_compile_features(temgi-sdl
    PRIVATE cxx_std_20
)

target_link_libraries(temgi-sdl
    PRIVATE
        TEMGI::Console
        SDL3::SDL3
)

