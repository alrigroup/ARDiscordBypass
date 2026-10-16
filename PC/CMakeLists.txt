cmake_minimum_required(VERSION 3.16)
project(ARDiscordBypass CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Alvo 1: ARDiscordBypass (CLI Padrão - Leve) ---
if (WIN32)
	add_custom_command(
		OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/proton_confgen_embed.o
		COMMAND ${CMAKE_OBJCOPY} -I binary -O pe-x86-64 -B i386:x86-64 proton-confgen.exe ${CMAKE_CURRENT_BINARY_DIR}/proton_confgen_embed.o
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/bin/proton-confgen.exe
		COMMENT "Embutindo binario Windows do proton-confgen.exe"
	)
else()
	add_custom_command(
		OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/proton_confgen_embed.o
		COMMAND objcopy -I binary -O elf64-x86-64 -B i386:x86-64 --rename-section .data=.rodata,alloc,load,readonly,data,contents proton-confgen ${CMAKE_CURRENT_BINARY_DIR}/proton_confgen_embed.o
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/bin/proton-confgen
		COMMENT "Embutindo binario Linux do proton-confgen"
	)
endif()

add_executable(ARDiscordBypass main.cpp ${CMAKE_CURRENT_BINARY_DIR}/proton_confgen_embed.o)
target_include_directories(ARDiscordBypass PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if (WIN32)
	target_link_libraries(ARDiscordBypass PRIVATE ws2_32 winhttp)
else()
	target_compile_definitions(ARDiscordBypass PRIVATE _POSIX_C_SOURCE=200809L)
	target_link_options(ARDiscordBypass PRIVATE -static-libgcc -static-libstdc++)
endif()

# --- Alvo 2: ARDiscordBypass_gui (GUI Opcional - Dear ImGui) ---
option(BUILD_GUI "Build the optional Graphical User Interface (Dear ImGui)" OFF)

if (BUILD_GUI)
	set(GUI_SOURCES
		gui/main.cpp
		gui/app_window.cpp
		gui/vendor/imgui/imgui.cpp
		gui/vendor/imgui/imgui_draw.cpp
		gui/vendor/imgui/imgui_tables.cpp
		gui/vendor/imgui/imgui_widgets.cpp
		gui/vendor/imgui/backends/imgui_impl_opengl3.cpp
	)

	if (WIN32)
		list(APPEND GUI_SOURCES gui/vendor/imgui/backends/imgui_impl_win32.cpp)
		add_executable(ARDiscordBypass_gui WIN32 ${GUI_SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/proton_confgen_embed.o)
		target_include_directories(ARDiscordBypass_gui PRIVATE 
			${CMAKE_CURRENT_SOURCE_DIR}
			${CMAKE_CURRENT_SOURCE_DIR}/gui
			${CMAKE_CURRENT_SOURCE_DIR}/gui/vendor/imgui
		)
		target_link_libraries(ARDiscordBypass_gui PRIVATE 
			opengl32 gdi32 user32 comdlg32 dwmapi imm32 ws2_32 winhttp
		)
	else()
		list(APPEND GUI_SOURCES gui/vendor/imgui/backends/imgui_impl_glfw.cpp)
		add_executable(ARDiscordBypass_gui ${GUI_SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/proton_confgen_embed.o)
		target_include_directories(ARDiscordBypass_gui PRIVATE 
			${CMAKE_CURRENT_SOURCE_DIR}
			${CMAKE_CURRENT_SOURCE_DIR}/gui
			${CMAKE_CURRENT_SOURCE_DIR}/gui/vendor/imgui
		)
		find_package(PkgConfig REQUIRED)
		pkg_check_modules(GLFW REQUIRED glfw3)
		find_package(OpenGL REQUIRED)

		target_link_libraries(ARDiscordBypass_gui PRIVATE 
			${GLFW_LIBRARIES} OpenGL::GL dl pthread
		)
		target_include_directories(ARDiscordBypass_gui PRIVATE ${GLFW_INCLUDE_DIRS})
		target_link_options(ARDiscordBypass_gui PRIVATE -static-libgcc -static-libstdc++)
	endif()
endif()
