#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <string>

#define ARDCB_GUI_BUILD
#include "../main.cpp"

#include "vendor/imgui/imgui.h"
#include "vendor/imgui/backends/imgui_impl_opengl3.h"
#include "style.h"
#include "app_window.h"

#define STB_IMAGE_IMPLEMENTATION
#include "vendor/stb_image.h"
#include "logo_data.h"

namespace fs = std::filesystem;

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

static bool shouldRunInCliMode(int argc, char *argv[]) {
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "-stop" || arg == "--stop" ||
        arg == "-cfg" || arg == "--cfg" || arg == "-c" || arg == "--config" ||
        arg.rfind("-cfg=", 0) == 0 || arg.rfind("--cfg=", 0) == 0 ||
        arg == "-l" || arg == "-login" || arg == "--login" || arg == "--proton" ||
        arg == "-h" || arg == "--help" || arg == "-help" ||
        arg == "--cli" || arg == "-cli") {
      return true;
    }
  }

#ifndef _WIN32
  const char *disp = std::getenv("DISPLAY");
  const char *wayl = std::getenv("WAYLAND_DISPLAY");
  if ((!disp || !*disp) && (!wayl || !*wayl)) {
    return true; // Terminal headless / SSH sem servidor gráfico
  }
#endif

  return false;
}

#ifdef _WIN32
#include <windows.h>
#include <tchar.h>
#include <GL/gl.h>
#include "vendor/imgui/backends/imgui_impl_win32.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
    return true;

  switch (msg) {
    case WM_SIZE:
      return 0;
    case WM_SYSCOMMAND:
      if ((wParam & 0xfff0) == SC_KEYMENU)
        return 0;
      break;
    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;
  }
  return DefWindowProcW(hWnd, msg, wParam, lParam);
}

static GLuint CreateLogoTexture() {
  int w = 0, h = 0, channels = 0;
  unsigned char *data = stbi_load_from_memory(logo_png, logo_png_len, &w, &h, &channels, 4);
  if (!data) return 0;

  GLuint tex = 0;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
  stbi_image_free(data);
  return tex;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
  if (shouldRunInCliMode(__argc, __argv)) {
    if (AttachConsole(ATTACH_PARENT_PROCESS)) {
      FILE *fp;
      freopen_s(&fp, "CONOUT$", "w", stdout);
      freopen_s(&fp, "CONOUT$", "w", stderr);
      freopen_s(&fp, "CONIN$", "r", stdin);
    }
    return runCliMain(__argc, __argv);
  }

  WNDCLASSEXW wc = {
      sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, hInstance,
      LoadIcon(NULL, IDI_APPLICATION), LoadCursor(NULL, IDC_ARROW),
      NULL, NULL, L"ARDiscordBypassGUI", NULL
  };
  RegisterClassExW(&wc);

  HWND hwnd = CreateWindowW(
      wc.lpszClassName, L"ARDiscordBypass v3.5.0 — ALRI GROUP",
      WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
      100, 100, 740, 580, NULL, NULL, wc.hInstance, NULL
  );

  PIXELFORMATDESCRIPTOR pfd = {
      sizeof(PIXELFORMATDESCRIPTOR), 1,
      PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
      PFD_TYPE_RGBA, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      24, 8, 0, PFD_MAIN_PLANE, 0, 0, 0, 0
  };

  HDC hdc = GetDC(hwnd);
  int pf = ChoosePixelFormat(hdc, &pfd);
  SetPixelFormat(hdc, pf, &pfd);
  HGLRC hglrc = wglCreateContext(hdc);
  wglMakeCurrent(hdc, hglrc);

  ShowWindow(hwnd, nCmdShow);
  UpdateWindow(hwnd);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

  // Carregar fontes limpas do sistema
  ImFontConfig fontCfg;
  fontCfg.OversampleH = 2;
  fontCfg.OversampleV = 1;
  const char *candidateFonts[] = {
      "C:\\Windows\\Fonts\\segoeui.ttf",
      "C:\\Windows\\Fonts\\tahoma.ttf",
      "C:\\Windows\\Fonts\\arial.ttf"
  };
  bool fontLoaded = false;
  for (const char *path : candidateFonts) {
    if (fs::exists(path)) {
      io.Fonts->AddFontFromFileTTF(path, 16.0f, &fontCfg, io.Fonts->GetGlyphRangesDefault());
      fontLoaded = true;
      break;
    }
  }
  if (!fontLoaded) io.Fonts->AddFontDefault();

  GUI::applyTheme(true);

  ImGui_ImplWin32_Init(hwnd);
  ImGui_ImplOpenGL3_Init("#version 130");

  GLuint logoTex = CreateLogoTexture();

  GUI::AppWindow app;
  app.setLogoTexture(logoTex);
  app.setDarkMode(true);

  bool done = false;
  while (!done) {
    MSG msg;
    while (PeekMessageW(&msg, NULL, 0U, 0U, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessageW(&msg);
      if (msg.message == WM_QUIT)
        done = true;
    }
    if (done) break;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    app.render();

    ImGui::Render();
    RECT rect;
    GetClientRect(hwnd, &rect);
    glViewport(0, 0, rect.right - rect.left, rect.bottom - rect.top);

    const GUI::ThemeColors &th = GUI::getTheme();
    glClearColor(th.bg.x, th.bg.y, th.bg.z, th.bg.w);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SwapBuffers(hdc);

    Sleep(16);
  }

  if (logoTex != 0) glDeleteTextures(1, &logoTex);

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplWin32_Shutdown();
  ImGui::DestroyContext();

  wglMakeCurrent(NULL, NULL);
  wglDeleteContext(hglrc);
  ReleaseDC(hwnd, hdc);
  DestroyWindow(hwnd);
  UnregisterClassW(wc.lpszClassName, wc.hInstance);

  return 0;
}

#else
// Linux Desktop com GLFW3 + OpenGL3
#include <GLFW/glfw3.h>
#include "vendor/imgui/backends/imgui_impl_glfw.h"

static GLuint CreateLogoTexture() {
  int w = 0, h = 0, channels = 0;
  unsigned char *data = stbi_load_from_memory(logo_png, logo_png_len, &w, &h, &channels, 4);
  if (!data) return 0;

  GLuint tex = 0;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
  stbi_image_free(data);
  return tex;
}

int main(int argc, char *argv[]) {
  if (shouldRunInCliMode(argc, argv)) {
    return runCliMain(argc, argv);
  }

  if (!glfwInit()) {
    std::fprintf(stderr, "Falha ao inicializar GLFW. Executando em modo CLI de fallback...\n");
    return runCliMain(argc, argv);
  }

  const char *glsl_version = "#version 130";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

  GLFWwindow *window = glfwCreateWindow(740, 580, "ARDiscordBypass v3.5.0 — ALRI GROUP", nullptr, nullptr);
  if (!window) {
    glfwTerminate();
    std::fprintf(stderr, "Não foi possível criar a janela gráfica. Executando em modo CLI de fallback...\n");
    return runCliMain(argc, argv);
  }
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

  // Carregar fontes limpas do sistema
  ImFontConfig fontCfg;
  fontCfg.OversampleH = 2;
  fontCfg.OversampleV = 1;
  const char *candidateFonts[] = {
      "/usr/share/fonts/TTF/DejaVuSans.ttf",
      "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
      "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
      "/usr/share/fonts/TTF/LiberationSans-Regular.ttf"
  };
  bool fontLoaded = false;
  for (const char *path : candidateFonts) {
    if (fs::exists(path)) {
      io.Fonts->AddFontFromFileTTF(path, 15.0f, &fontCfg, io.Fonts->GetGlyphRangesDefault());
      fontLoaded = true;
      break;
    }
  }
  if (!fontLoaded) io.Fonts->AddFontDefault();

  GUI::applyTheme(true);

  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  GLuint logoTex = CreateLogoTexture();

  GUI::AppWindow app;
  app.setLogoTexture(logoTex);
  app.setDarkMode(true);

  while (!glfwWindowShouldClose(window)) {
    if (app.isAnimating()) {
      glfwPollEvents();
    } else {
      glfwWaitEventsTimeout(0.03);
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    app.render();

    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);

    const GUI::ThemeColors &th = GUI::getTheme();
    glClearColor(th.bg.x, th.bg.y, th.bg.z, th.bg.w);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
  }

  if (logoTex != 0) glDeleteTextures(1, &logoTex);

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
#endif
