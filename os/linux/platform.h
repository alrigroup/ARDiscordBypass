#pragma once

#include <array>
#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace fs = std::filesystem;

inline std::string runCommand(const std::string &cmd) {
  std::array<char, 256> buffer;
  std::string result;
  FILE *pipe = popen(cmd.c_str(), "r");
  if (!pipe)
    return "";
  while (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
    result += buffer.data();
  pclose(pipe);
  while (!result.empty() && (result.back() == '\n' || result.back() == '\r'))
    result.pop_back();
  return result;
}

inline bool isDiscordRunning() {
  return std::system("pgrep -x Discord >/dev/null 2>&1") == 0;
}

inline void killDiscord() {
  std::system("pkill -TERM -x Discord >/dev/null 2>&1");
  std::this_thread::sleep_for(std::chrono::seconds(1));
  std::system("pkill -KILL -x Discord >/dev/null 2>&1");
}

inline std::string findDiscordExeIn(const fs::path &basePath) {
  const fs::path discordRoot = basePath / "discord";
  if (!fs::exists(discordRoot))
    return "";

  fs::path latestPath;
  std::string latestVersion;
  for (const auto &entry : fs::directory_iterator(discordRoot)) {
    if (!entry.is_directory())
      continue;
    const std::string folder = entry.path().filename().string();
    if (folder.rfind("app-", 0) != 0)
      continue;
    const fs::path exePath = entry.path() / "Discord";
    if (fs::is_regular_file(exePath) &&
        (latestVersion.empty() || folder > latestVersion)) {
      latestVersion = folder;
      latestPath = exePath;
    }
  }
  return latestPath.empty() ? "" : latestPath.string();
}

inline std::string findDiscordExeFlatpak() {
  const char *home = std::getenv("HOME");
  const std::vector<fs::path> flatpakBases = {
      "/var/lib/flatpak/app/com.discordapp.Discord",
      std::string(home ? home : "") +
          "/.local/share/flatpak/app/com.discordapp.Discord"};

  for (const auto &base : flatpakBases) {
    if (!fs::exists(base))
      continue;
    const fs::path stablePath = base / "x86_64" / "stable";
    if (!fs::exists(stablePath))
      continue;

    const fs::path activePath =
        stablePath / "active" / "files" / "discord" / "Discord";
    if (fs::is_regular_file(activePath))
      return activePath.string();

    for (const auto &entry : fs::directory_iterator(stablePath)) {
      if (!entry.is_directory())
        continue;
      const std::string folder = entry.path().filename().string();
      if (folder == "active" || folder == ".ref")
        continue;
      const fs::path exePath = entry.path() / "files" / "discord" / "Discord";
      if (fs::is_regular_file(exePath))
        return exePath.string();
    }
  }
  return "";
}

inline std::string findDiscordExeSnap() {
  const std::vector<fs::path> snapPaths = {"/snap/discord/current",
                                            "/var/lib/snapd/snap/discord/current"};

  for (const auto &path : snapPaths) {
    const fs::path exePath = path / "Discord";
    if (fs::is_regular_file(exePath))
      return exePath.string();
  }
  return "";
}

inline std::string findDiscordExeSystem() {
  const std::vector<fs::path> exeLocations = {
      "/usr/bin/Discord",     "/usr/bin/discord",
      "/usr/local/bin/Discord", "/usr/local/bin/discord",
      "/opt/Discord/Discord",  "/opt/discord/Discord",
      "/opt/Discord/discord",  "/opt/discord/discord",
      "/usr/share/discord/Discord", "/usr/share/discord/discord",
      "/usr/lib/discord/Discord",   "/usr/lib/discord/discord",
      "/usr/lib64/discord/Discord"};

  for (const auto &path : exeLocations) {
    if (fs::is_regular_file(path))
      return path.string();
  }

  const std::vector<fs::path> systemDirs = {"/opt/Discord",
                                             "/opt/discord",
                                             "/usr/share/discord",
                                             "/usr/lib/discord",
                                             "/usr/lib64/discord"};

  for (const auto &dir : systemDirs) {
    if (!fs::exists(dir))
      continue;
    for (const auto &entry : fs::directory_iterator(dir)) {
      if (!entry.is_directory())
        continue;
      const std::string folder = entry.path().filename().string();
      if (folder.rfind("app-", 0) != 0)
        continue;
      const fs::path exePath = entry.path() / "Discord";
      if (fs::is_regular_file(exePath))
        return exePath.string();
    }
  }

  return "";
}

inline std::string findDiscordExeXdg() {
  const char *home = std::getenv("HOME");
  if (!home)
    return "";

  const char *configHome = std::getenv("XDG_CONFIG_HOME");
  const fs::path configPath =
      configHome && *configHome ? fs::path(configHome) : fs::path(home) / ".config";

  std::string result = findDiscordExeIn(configPath);
  if (!result.empty())
    return result;

  const char *dataHome = std::getenv("XDG_DATA_HOME");
  const fs::path dataPath = dataHome && *dataHome
                                ? fs::path(dataHome)
                                : fs::path(home) / ".local" / "share";
  result = findDiscordExeIn(dataPath);
  if (!result.empty())
    return result;

  const std::vector<fs::path> xdgDataDirs = {"/usr/local/share", "/usr/share"};

  for (const auto &dir : xdgDataDirs) {
    result = findDiscordExeIn(dir);
    if (!result.empty())
      return result;
  }

  return "";
}

inline std::string findDiscordExeWhich() {
  std::string path = runCommand("which discord 2>/dev/null");
  if (!path.empty() && fs::is_regular_file(path))
    return path;

  path = runCommand("which Discord 2>/dev/null");
  if (!path.empty() && fs::is_regular_file(path))
    return path;

  path = runCommand("command -v discord 2>/dev/null");
  if (!path.empty() && fs::is_regular_file(path))
    return path;

  return "";
}

inline bool isFlatpakPath(const std::string &path) {
  return path.find("/flatpak/") != std::string::npos ||
         path.find("/.local/share/flatpak/") != std::string::npos;
}

inline bool isSnapPath(const std::string &path) {
  return path.find("/snap/") != std::string::npos ||
         path.find("/snapd/") != std::string::npos;
}

inline std::string getDiscordFlagsPath(const std::string &exePath) {
  const char *home = std::getenv("HOME");
  if (!home)
    return "";

  if (isFlatpakPath(exePath))
    return std::string(home) +
           "/.var/app/com.discordapp.Discord/config/discord-flags.conf";

  if (isSnapPath(exePath))
    return std::string(home) + "/snap/discord/common/discord-flags.conf";

  return std::string(home) + "/.config/discord-flags.conf";
}

inline bool writeDiscordFlags(const std::string &flagsPath,
                               const std::string &proxyEndpoint) {
  const std::string bypassList =
      "cdn.discordapp.com;*.discordapp.net;*.discord.media;<local>";

  fs::path dir = fs::path(flagsPath).parent_path();
  if (!dir.empty() && !fs::exists(dir)) {
    std::error_code ec;
    fs::create_directories(dir, ec);
    if (ec)
      return false;
  }

  std::ofstream ofs(flagsPath);
  if (!ofs.is_open())
    return false;

  ofs << "--proxy-server=" << proxyEndpoint << "\n";
  ofs << "--proxy-bypass-list=" << bypassList << "\n";
  ofs.close();
  return true;
}

inline void removeDiscordFlags(const std::string &flagsPath) {
  if (!flagsPath.empty() && fs::exists(flagsPath)) {
    std::error_code ec;
    fs::remove(flagsPath, ec);
  }
}

inline std::string locateDiscordExe() {
  const char *home = std::getenv("HOME");
  if (!home)
    return "";

  std::string result;

  result = findDiscordExeWhich();
  if (!result.empty())
    return result;

  result = findDiscordExeFlatpak();
  if (!result.empty())
    return result;

  result = findDiscordExeSnap();
  if (!result.empty())
    return result;

  result = findDiscordExeXdg();
  if (!result.empty())
    return result;

  result = findDiscordExeSystem();
  if (!result.empty())
    return result;

  return "";
}

inline bool launchDiscord(const std::string &exePath,
                           const std::string &proxyEndpoint) {
  const std::string flagsPath = getDiscordFlagsPath(exePath);

  removeDiscordFlags(flagsPath);

  if (!proxyEndpoint.empty() &&
      (isFlatpakPath(exePath) || isSnapPath(exePath))) {
    if (!writeDiscordFlags(flagsPath, proxyEndpoint)) {
      std::cerr << "AVISO: Não consegui criar discord-flags.conf em "
                << flagsPath << std::endl;
    }
  }

  const pid_t child = fork();
  if (child < 0)
    return false;
  if (child == 0) {
    int devNull = open("/dev/null", O_RDWR);
    if (devNull >= 0) {
      dup2(devNull, STDOUT_FILENO);
      dup2(devNull, STDERR_FILENO);
      close(devNull);
    }

    const std::string bypassList =
        "cdn.discordapp.com;*.discordapp.net;*.discord.media;<local>";

    if (isFlatpakPath(exePath)) {
      execl("/usr/bin/flatpak", "flatpak", "run", "com.discordapp.Discord",
            nullptr);
    } else if (isSnapPath(exePath)) {
      execl("/usr/bin/snap", "snap", "run", "discord", nullptr);
    } else if (!proxyEndpoint.empty()) {
      execl(exePath.c_str(), exePath.c_str(),
            ("--proxy-server=" + proxyEndpoint).c_str(),
            ("--proxy-bypass-list=" + bypassList).c_str(), nullptr);
    } else {
      execl(exePath.c_str(), exePath.c_str(), nullptr);
    }
    _exit(127);
  }

  int status;
  waitpid(child, &status, WNOHANG);
  std::this_thread::sleep_for(std::chrono::seconds(3));

  removeDiscordFlags(flagsPath);
  return true;
}

inline std::vector<std::string> fetchProxies() {
  std::vector<std::string> proxies;
  std::string data = runCommand(
      "curl -s --max-time 10 "
      "\"https://api.proxyscrape.com/v4/free-proxy-list/get?"
      "request=display_proxies&proxy_format=protocolipport"
      "&format=text&protocol=socks5&timeout=5000\"");
  if (data.empty())
    return proxies;

  std::istringstream stream(data);
  std::string line;
  while (std::getline(stream, line)) {
    while (!line.empty() && (line.back() == '\r' || line.back() == '\n'))
      line.pop_back();
    if (!line.empty())
      proxies.push_back(line);
  }
  return proxies;
}

inline bool testProxy(const std::string &proxy) {
  std::string result = runCommand("curl -s --max-time 8 --socks5-hostname " +
                                   proxy +
                                   " https://discord.com/api/v10/gateway "
                                   "2>/dev/null");
  if (result.empty())
    return false;
  return result.find("url") != std::string::npos ||
         result.find("wss://") != std::string::npos;
}

inline std::string findWorkingProxy() {
  std::vector<std::string> proxies = fetchProxies();
  if (proxies.empty())
    return "";

  for (size_t i = 0; i < proxies.size() && i < 15; ++i) {
    if (testProxy(proxies[i]))
      return proxies[i];
  }
  return "";
}
