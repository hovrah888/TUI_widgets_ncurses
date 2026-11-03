cmake_minimum_required(VERSION 3.16)
project(console_widgets LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# compile_commands.json для clangd / IDE
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Curses REQUIRED)

# --- библиотека виджетов ---
add_library(hcm hcm.cpp hcm.h)
target_include_directories(hcm PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
if(CURSES_INCLUDE_DIRS)
    target_include_directories(hcm PUBLIC ${CURSES_INCLUDE_DIRS})
endif()
target_link_libraries(hcm PUBLIC ${CURSES_LIBRARIES})
target_compile_options(hcm PRIVATE -Wall -Wextra -Wpedantic)

# --- примеры ---
add_executable(demo main.cpp)
target_link_libraries(demo PRIVATE hcm)

add_executable(example1 example1.cpp)
target_link_libraries(example1 PRIVATE hcm)

# --- опциональные санитайзеры ---
option(HCM_SANITIZE "Build with AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
if(HCM_SANITIZE)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()
