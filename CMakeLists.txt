cmake_minimum_required(VERSION 3.12)
project(SmartRoute VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include third_party)

set(COMMON_SOURCES
    src/Graph.cpp
    src/Algorithms.cpp
    src/Navigation.cpp
)

# 1. CLI Terminal Executable
add_executable(smartroute
    src/main.cpp
    src/UI.cpp
    ${COMMON_SOURCES}
)

# 2. Web Server Executable
add_executable(smartroute_web
    src/server_main.cpp
    src/Server.cpp
    ${COMMON_SOURCES}
)

find_package(Threads REQUIRED)
target_link_libraries(smartroute_web PRIVATE Threads::Threads)

if (MSVC)
    target_compile_options(smartroute PRIVATE /W4)
    target_compile_options(smartroute_web PRIVATE /W4)
else()
    target_compile_options(smartroute PRIVATE -Wall -Wextra -O2)
    target_compile_options(smartroute_web PRIVATE -Wall -Wextra -O2)
endif()
