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
add_executable(EppoChatClient ${SOURCES})

target_include_directories(EppoChatClient PRIVATE ${INCLUDE_DIRS})
target_link_libraries(EppoChatClient PRIVATE EppoCore GameNetworkingSockets)