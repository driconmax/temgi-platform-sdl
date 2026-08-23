cmake_minimum_required(VERSION 3.20)

project(
    temgi-platform-sdl
    VERSION 0.1.0
    LANGUAGES CXX
)

find_package(SDL3 CONFIG REQUIRED)

set(
    TEMGI_SDK
    ${CMAKE_CURRENT_SOURCE_DIR}/sdk/temgi
)

add_executable(
    temgi-sdl

    src/main.cpp
    src/SDLPlatform.cpp
    src/SDLInput.cpp
    src/SDLVideo.cpp
)

target_compile_features(
    temgi-sdl
    PRIVATE
        cxx_std_20
)

target_include_directories(
    temgi-sdl
    PRIVATE
        ${TEMGI_SDK}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(
    temgi-sdl
    PRIVATE
        ${TEMGI_SDK}/lib/libtemgi.a
        SDL3::SDL3
)

# Export TEMGI symbols so dynamically loaded cartridges
# can resolve Console/Graphics/Input/etc. from this executable.
set_target_properties(
    temgi-sdl
    PROPERTIES
        ENABLE_EXPORTS ON
)