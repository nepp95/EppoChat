cmake_minimum_required(VERSION 3.14)
project(EppoChatServer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Files
file(GLOB_RECURSE SOURCES "Source/*.cpp")
set(INCLUDE_DIRS
    "Source"
    "${CMAKE_SOURCE_DIR}/EppoChatCommon/Source"
)

# Create target
add_executable(EppoChatServer ${SOURCES})
target_include_directories(EppoChatServer PRIVATE ${INCLUDE_DIRS})
target_link_libraries(EppoChatServer PRIVATE EppoCore_Headless GameNetworkingSockets::shared)
target_compile_definitions(EppoChatServer PRIVATE EP_HEADLESS)
