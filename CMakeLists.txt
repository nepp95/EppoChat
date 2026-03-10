cmake_minimum_required(VERSION 3.14)
project(EppoChat)

# General options
set(CMAKE_CONFIGURATION_TYPES Debug Release)

if (MSVC)
    add_compile_options("/MP")
endif ()

# Dependencies
include(FetchContent)

FetchContent_Declare(
    EppoCore
    GIT_REPOSITORY https://github.com/nepp95/EppoCore.git
    GIT_TAG 7cde3c50cacd986a19ecb7366200c70bc4f28267
)

FetchContent_MakeAvailable(EppoCore)

find_package(GameNetworkingSockets CONFIG REQUIRED)

# Targets
add_subdirectory(EppoChatClient)
add_subdirectory(EppoChatServer)
