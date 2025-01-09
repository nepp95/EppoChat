project(EppoChatClient VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
target_link_libraries(EppoChatClient PRIVATE EppoCore_GUI GameNetworkingSockets)

add_custom_command(
	TARGET EppoChatClient POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/bin/GameNetworkingSockets.dll" "${CMAKE_BINARY_DIR}/EppoChatClient/GameNetworkingSockets.dll"
	COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/bin/libcrypto-3-x64.dll" "${CMAKE_BINARY_DIR}/EppoChatClient/libcrypto-3-x64.dll"
	COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/bin/libprotobufd.dll" "${CMAKE_BINARY_DIR}/EppoChatClient/libprotobufd.dll"
)
