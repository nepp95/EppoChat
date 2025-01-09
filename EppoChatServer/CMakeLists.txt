project(EppoChatServer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Files
file(GLOB_RECURSE SOURCES "Source/*.cpp")

set(INCLUDE_DIRS
    "Source"
    "../Vendor/EppoCore/EppoCore/Source"
    "../Vendor/EppoCore/EppoCore/Vendor"
    "../Vendor/EppoCore/EppoCore/Vendor/spdlog/include"
    "../Vendor/GameNetworkingSockets/include"
)

# Create target
add_executable(EppoChatServer ${SOURCES})

if (CMAKE_HOST_LINUX)
    target_compile_definitions(EppoChatServer PUBLIC EPPO_LINUX)
elseif (CMAKE_HOST_WIN32)
    target_compile_definitions(EppoChatServer PUBLIC EPPO_WINDOWS)
endif()

target_include_directories(EppoChatServer PRIVATE ${INCLUDE_DIRS})
target_link_libraries(EppoChatServer PRIVATE EppoCore_Headless GameNetworkingSockets)
target_compile_definitions(EppoChatServer PRIVATE EPPO_HEADLESS)

add_custom_command(
	TARGET EppoChatServer POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/bin/GameNetworkingSockets.dll" "${CMAKE_BINARY_DIR}/EppoChatServer/GameNetworkingSockets.dll"
	COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/bin/libcrypto-3-x64.dll" "${CMAKE_BINARY_DIR}/EppoChatServer/libcrypto-3-x64.dll"
	COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/bin/libprotobufd.dll" "${CMAKE_BINARY_DIR}/EppoChatServer/libprotobufd.dll"
)
