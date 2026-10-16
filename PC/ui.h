#pragma once

#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <atomic>
#include <future>
#include <vector>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

namespace UI {

// Paleta ANSI moderna com suporte a TrueColor e fallback padrão
inline const char *RESET       = "\033[0m";
inline const char *BOLD        = "\033[1m";
inline const char *DIM         = "\033[2m";
inline const char *ITALIC      = "\033[3m";
inline const char *UNDERLINE   = "\033[4m";

// Cores ALRI e Modern Terminal (Vermelho puro e marcante #E50914)
inline const char *RED         = "\033[38;2;229;9;20m";
inline const char *RED_BRIGHT  = "\033[38;2;255;30;39m";
inline const char *RED_DARK    = "\033[38;2;178;7;16m";
inline const char *GREEN       = "\033[38;2;0;230;118m";
inline const char *YELLOW      = "\033[38;2;255;214;0m";
inline const char *CYAN        = "\033[38;2;0;212;255m";
inline const char *WHITE       = "\033[38;2;248;250;252m";
inline const char *MUTED       = "\033[38;2;100;116;139m";

inline int getTerminalWidth() {
  int width = 80;
#ifdef _WIN32
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
  if (hOut != INVALID_HANDLE_VALUE && GetConsoleScreenBufferInfo(hOut, &csbi)) {
    width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
  }
#else
  struct winsize w;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0 && w.ws_col > 0) {
    width = w.ws_col;
  }
#endif
  if (width < 20) width = 80;
  return width;
}

inline void initConsole() {
#ifdef _WIN32
  SetConsoleOutputCP(CP_UTF8);
  HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
  if (hOut != INVALID_HANDLE_VALUE) {
    DWORD dwMode = 0;
    if (GetConsoleMode(hOut, &dwMode)) {
      dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
      SetConsoleMode(hOut, dwMode);
    }
  }
#endif
}

inline void clearLine() {
  std::cout << "\33[2K\r" << std::flush;
}

inline void info(const std::string &msg) {
  std::cout << "  " << CYAN << "ℹ" << RESET << "  " << msg << std::endl;
}

inline void success(const std::string &msg) {
  std::cout << "  " << GREEN << "✔" << RESET << "  " << BOLD << msg << RESET << std::endl;
}

inline void warn(const std::string &msg) {
  std::cout << "  " << YELLOW << "⚠" << RESET << "  " << msg << std::endl;
}

inline void error(const std::string &msg) {
  std::cout << "  " << RED << "✖" << RESET << "  " << BOLD << RED << msg << RESET << std::endl;
}

// Loading animado de alta precisão com Braille dots e efeito glow
template <typename Func>
auto spinner(const std::string &message, Func &&func) {
  std::atomic<bool> done{false};
  const std::vector<std::string> frames = {
      "⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"
  };

  auto future = std::async(std::launch::async, [&func, &done]() {
    if constexpr (std::is_void_v<std::invoke_result_t<Func>>) {
      func();
      done = true;
    } else {
      auto res = func();
      done = true;
      return res;
    }
  });

  size_t idx = 0;
  while (!done) {
    std::cout << "\r  " << RED << frames[idx % frames.size()] << RESET << "  "
              << WHITE << message << RESET << " "
              << MUTED << "..." << RESET << std::flush;
    std::this_thread::sleep_for(std::chrono::milliseconds(70));
    idx++;
  }

  clearLine();

  if constexpr (std::is_void_v<std::invoke_result_t<Func>>) {
    future.get();
    std::cout << "  " << GREEN << "✔" << RESET << "  " << BOLD << message << RESET << std::endl;
  } else {
    auto res = future.get();
    if constexpr (std::is_same_v<decltype(res), bool>) {
      if (res) {
        std::cout << "  " << GREEN << "✔" << RESET << "  " << BOLD << message << RESET << std::endl;
      } else {
        std::cout << "  " << RED << "✖" << RESET << "  " << BOLD << RED << message << " (Falhou)" << RESET << std::endl;
      }
    } else {
      std::cout << "  " << GREEN << "✔" << RESET << "  " << BOLD << message << RESET << std::endl;
    }
    return res;
  }
}

// Barra de progresso animada com porcentagem
inline void progressBar(const std::string &title, int durationMs = 2000) {
  int termWidth = getTerminalWidth();
  int barWidth = termWidth > 60 ? 30 : 16;
  int steps = 100;
  int sleepPerStep = durationMs / steps;

  for (int i = 0; i <= steps; ++i) {
    int filled = (i * barWidth) / 100;
    std::cout << "\r  " << RED << "⚡" << RESET << " " << WHITE << title << RESET << " [" << RED;
    for (int j = 0; j < filled; ++j) std::cout << "█";
    std::cout << MUTED;
    for (int j = filled; j < barWidth; ++j) std::cout << "░";
    std::cout << RESET << "] " << BOLD << i << "%" << RESET << std::flush;
    std::this_thread::sleep_for(std::chrono::milliseconds(sleepPerStep));
  }
  clearLine();
  std::cout << "  " << GREEN << "✔" << RESET << "  " << BOLD << title << " [100%]" << RESET << std::endl;
}

} // namespace UI
