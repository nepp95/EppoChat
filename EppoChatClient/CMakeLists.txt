cmake_minimum_required(VERSION 3.14)
project(EppoChatClient VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Files
file(GLOB_RECURSE SOURCES "Source/*.cpp")
set(INCLUDE_DIRS
    "Source"
    "${CMAKE_SOURCE_DIR}/EppoChatCommon/Source"
)

# Create target
add_executable(EppoChatClient ${SOURCES})
target_include_directories(EppoChatClient PRIVATE ${INCLUDE_DIRS})
target_link_libraries(EppoChatClient PRIVATE EppoCore_GUI GameNetworkingSockets::shared)
