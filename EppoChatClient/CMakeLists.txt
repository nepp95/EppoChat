set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

# Files
file(GLOB_RECURSE SOURCES "Source/*.cpp")

set(INCLUDE_DIRS
    "Source"
)

# Create target
add_executable(EppoChatClient ${SOURCES})

target_include_directories(EppoChatClient PRIVATE ${INCLUDE_DIRS})
target_link_libraries(EppoChatClient PRIVATE EppoChatCommon)