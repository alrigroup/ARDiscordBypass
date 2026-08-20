cmake_minimum_required(VERSION 3.16)
project(ARDiscordBypass_CPP CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(ARDiscordBypass_cpp main.cpp)

target_link_libraries(ARDiscordBypass_cpp PRIVATE ws2_32 winhttp)
