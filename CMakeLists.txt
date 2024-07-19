cmake_minimum_required(VERSION 3.14)

# Project
project(EppoChat VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Files
file(GLOB_RECURSE SOURCES "EppoChat/Source/*.cpp")

set(INCLUDE_DIRS
    "EppoChat/Source"
    "EppoCore/EppoCore/Source"
    "EppoCore/EppoCore/Vendor"
    "EppoCore/EppoCore/Vendor/glm"
    "EppoCore/EppoCore/Vendor/imgui"
    "EppoCore/EppoCore/Vendor/spdlog/include"
)

# Create target
add_executable(${PROJECT_NAME} ${SOURCES})
add_subdirectory(EppoCore)

target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE
    EppoCore
    glad
    glfw
    imgui
)