cmake_minimum_required(VERSION 3.16)
project(ARDiscordBypass CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(ARDiscordBypass main.cpp)

target_include_directories(ARDiscordBypass PRIVATE ${CMAKE_SOURCE_DIR})

if (WIN32)
	target_link_libraries(ARDiscordBypass PRIVATE ws2_32 winhttp)
else()
	target_compile_definitions(ARDiscordBypass PRIVATE _POSIX_C_SOURCE=200809L)
endif()
