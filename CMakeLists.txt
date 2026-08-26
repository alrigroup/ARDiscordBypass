cmake_minimum_required(VERSION 3.16)
project(ARDiscordBypass_CPP CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(ARDiscordBypass_cpp main.cpp)

if (WIN32)
	target_link_libraries(ARDiscordBypass_cpp PRIVATE ws2_32 winhttp)
else()
	target_compile_definitions(ARDiscordBypass_cpp PRIVATE _POSIX_C_SOURCE=200809L)
endif()
