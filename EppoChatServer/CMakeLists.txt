set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

# Files
file(GLOB_RECURSE SOURCES "Source/*.cpp")

set(INCLUDE_DIRS
    "Source"
    "../Vendor/EppoCore/EppoCore/Source"
    "../Vendor/EppoCore/EppoCore/Vendor"
    "../Vendor/EppoCore/EppoCore/Vendor/glm"
    "../Vendor/EppoCore/EppoCore/Vendor/imgui"
    "../Vendor/EppoCore/EppoCore/Vendor/spdlog/include"
    "../Vendor/GameNetworkingSockets/include"
)

# Create target
add_executable(EppoChatServer ${SOURCES})

if (CMAKE_HOST_LINUX)
    target_compile_definitions(EppoChatCommon PUBLIC EPPO_LINUX)
elseif (CMAKE_HOST_WIN32)
    target_compile_definitions(EppoChatCommon PUBLIC EPPO_WINDOWS)
endif()

target_include_directories(EppoChatServer PRIVATE ${INCLUDE_DIRS})
target_link_libraries(EppoChatServer PRIVATE EppoCore GameNetworkingSockets)
target_compile_definitions(EppoChatServer PRIVATE EPPO_HEADLESS)