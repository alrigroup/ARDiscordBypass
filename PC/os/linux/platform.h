#pragma once

#include <array>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace fs = std::filesystem;

inline const std::string NETNS_NAME = "ar-discord-vpn";
inline const std::string WG_IF = "wg-ardc";

enum class LaunchTargetType {
  NativeExe,
  Flatpak,
  Snap,
  Command
};

struct LaunchTarget {
  LaunchTargetType type = LaunchTargetType::NativeExe;
  std::string command;
  std::string displayName;
};

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

inline bool haveCommand(const std::string &binary) {
  return std::system(("command -v " + binary + " >/dev/null 2>&1").c_str()) == 0;
}

inline std::string detectLinuxDistro() {
  std::ifstream ofs("/etc/os-release");
  if (!ofs.is_open())
    return "linux";

  std::string line;
  std::string id, idLike;
  while (std::getline(ofs, line)) {
    if (line.rfind("ID=", 0) == 0) {
      id = line.substr(3);
      if (!id.empty() && id.front() == '"') id = id.substr(1, id.size() - 2);
    } else if (line.rfind("ID_LIKE=", 0) == 0) {
      idLike = line.substr(8);
      if (!idLike.empty() && idLike.front() == '"') idLike = idLike.substr(1, idLike.size() - 2);
    }
  }

  std::string combined = id + " " + idLike;
  for (char &c : combined) c = static_cast<char>(std::tolower(c));
  return combined;
}

inline bool checkPrerequisites(std::string &errorMessage) {
  bool missingIp = !haveCommand("ip");
  bool missingWg = !haveCommand("wg");

  if (missingIp || missingWg) {
    std::string distro = detectLinuxDistro();
    std::ostringstream ss;
    ss << "Dependências do sistema ausentes: ";
    if (missingIp) ss << "[iproute2 (ip)] ";
    if (missingWg) ss << "[wireguard-tools (wg)] ";
    ss << "\nInstale-as com o gerenciador de pacotes da sua distro:\n";

    if (distro.find("arch") != std::string::npos || distro.find("manjaro") != std::string::npos || distro.find("endeavour") != std::string::npos) {
      ss << "  sudo pacman -S --needed wireguard-tools iproute2";
    } else if (distro.find("debian") != std::string::npos || distro.find("ubuntu") != std::string::npos || distro.find("mint") != std::string::npos || distro.find("pop") != std::string::npos) {
      ss << "  sudo apt update && sudo apt install -y wireguard-tools iproute2";
    } else if (distro.find("fedora") != std::string::npos || distro.find("rhel") != std::string::npos) {
      ss << "  sudo dnf install -y wireguard-tools iproute";
    } else if (distro.find("suse") != std::string::npos) {
      ss << "  sudo zypper install -y wireguard-tools iproute2";
    } else {
      ss << "  Instale os pacotes 'wireguard-tools' e 'iproute2'.";
    }

    errorMessage = ss.str();
    return false;
  }

  return true;
}

inline bool isDiscordRunning() {
  return std::system("pgrep -x \"(Discord|DiscordCanary|DiscordPTB|vesktop|equibop)\" >/dev/null 2>&1") == 0;
}

inline bool isDiscordRunningInNetns() {
  std::string pid = runCommand("pgrep -x \"(Discord|DiscordCanary|DiscordPTB|vesktop|equibop)\" | head -1");
  if (pid.empty())
    return false;
  std::string ns = runCommand("sudo ip netns identify " + pid + " 2>/dev/null");
  return ns == NETNS_NAME;
}

inline void killDiscord() {
  std::system("pkill -TERM -x \"(Discord|DiscordCanary|DiscordPTB|vesktop|equibop)\" >/dev/null 2>&1");
  for (int i = 0; i < 10; ++i) {
    if (!isDiscordRunning())
      return;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }
}

inline std::string findDiscordExeInDir(const fs::path &dir, const std::string &binaryName = "Discord") {
  if (!fs::exists(dir))
    return "";

  fs::path latestPath;
  std::string latestVersion;
  try {
    for (const auto &entry : fs::directory_iterator(dir)) {
      if (!entry.is_directory())
        continue;
      const std::string folder = entry.path().filename().string();
      if (folder.rfind("app-", 0) != 0)
        continue;
      const fs::path exePath = entry.path() / binaryName;
      if (fs::is_regular_file(exePath) &&
          (latestVersion.empty() || folder > latestVersion)) {
        latestVersion = folder;
        latestPath = exePath;
      }
    }
  } catch (...) {}
  return latestPath.empty() ? "" : latestPath.string();
}

inline LaunchTarget locateDiscordLaunchTarget(const std::string &manualPath = "") {
  if (!manualPath.empty()) {
    if (fs::is_regular_file(manualPath)) {
      return {LaunchTargetType::NativeExe, manualPath, "Discord (Caminho Manual)"};
    }
    return {LaunchTargetType::Command, manualPath, "Discord (Comando Manual)"};
  }

  const char *home = std::getenv("HOME");
  std::string homeStr = home ? home : "";

  // 1. Procurar versões nativas oficiais na pasta do usuário ~/.config/discord*
  if (!homeStr.empty()) {
    std::string exe = findDiscordExeInDir(fs::path(homeStr) / ".config" / "discord", "Discord");
    if (!exe.empty()) return {LaunchTargetType::NativeExe, exe, "Discord Stable (Nativo)"};

    exe = findDiscordExeInDir(fs::path(homeStr) / ".config" / "discordcanary", "DiscordCanary");
    if (!exe.empty()) return {LaunchTargetType::NativeExe, exe, "Discord Canary (Nativo)"};

    exe = findDiscordExeInDir(fs::path(homeStr) / ".config" / "discordptb", "DiscordPTB");
    if (!exe.empty()) return {LaunchTargetType::NativeExe, exe, "Discord PTB (Nativo)"};
  }

  // 2. Procurar executáveis nativos no PATH do sistema
  const std::vector<std::pair<std::string, std::string>> binCandidates = {
      {"discord", "Discord Stable"},
      {"Discord", "Discord Stable"},
      {"discord-canary", "Discord Canary"},
      {"discordcanary", "Discord Canary"},
      {"discord-ptb", "Discord PTB"},
      {"discordptb", "Discord PTB"},
      {"vesktop", "Vesktop (Cliente Vencord)"},
      {"equibop", "Equibop (Cliente Equicord)"}
  };

  for (const auto &[cmd, name] : binCandidates) {
    if (haveCommand(cmd)) {
      std::string path = runCommand("command -v " + cmd + " 2>/dev/null");
      if (!path.empty()) {
        return {LaunchTargetType::NativeExe, path, name};
      }
    }
  }

  // 3. Procurar instalações Flatpak (do usuário ou do sistema)
  if (haveCommand("flatpak")) {
    const std::vector<std::pair<std::string, std::string>> flatpakApps = {
        {"com.discordapp.Discord", "Discord (Flatpak)"},
        {"com.discordapp.DiscordCanary", "Discord Canary (Flatpak)"},
        {"com.discordapp.DiscordPTB", "Discord PTB (Flatpak)"},
        {"dev.vencord.Vesktop", "Vesktop (Flatpak)"},
        {"org.equicord.equibop", "Equibop (Flatpak)"}
    };

    for (const auto &[appId, name] : flatpakApps) {
      std::string check = runCommand("flatpak info " + appId + " 2>/dev/null");
      if (!check.empty()) {
        return {LaunchTargetType::Flatpak, "flatpak run " + appId, name};
      }
    }
  }

  // 4. Procurar instalações Snap
  if (haveCommand("snap")) {
    if (fs::is_regular_file("/snap/bin/discord") || fs::is_regular_file("/var/lib/snapd/snap/bin/discord")) {
      return {LaunchTargetType::Snap, "snap run discord", "Discord (Snap)"};
    }
  }

  // 5. Procurar diretórios de sistema padrão (/opt, /usr/share, /usr/lib)
  const std::vector<std::string> systemDirs = {
      "/opt/discord/Discord", "/opt/Discord/Discord",
      "/usr/share/discord/Discord", "/usr/lib/discord/Discord"
  };
  for (const auto &sysPath : systemDirs) {
    if (fs::is_regular_file(sysPath)) {
      return {LaunchTargetType::NativeExe, sysPath, "Discord (Sistema)"};
    }
  }

  return {};
}

inline bool ensureWireGuardNetns() {
  std::system("sudo modprobe wireguard >/dev/null 2>&1 || true");

  std::string netnsCheck = runCommand("ip netns list 2>/dev/null | grep -w " + NETNS_NAME);
  bool netnsExists = !netnsCheck.empty();

  if (netnsExists) {
    std::string ipCheck = runCommand("sudo ip netns exec " + NETNS_NAME + " curl -s -m 3 https://api.ipify.org 2>/dev/null");
    if (!ipCheck.empty()) {
      return true;
    }
    std::system(("sudo ip netns del " + NETNS_NAME + " 2>/dev/null").c_str());
  }

  std::system(("sudo ip link del dev " + WG_IF + " 2>/dev/null").c_str());
  std::system(("sudo ip netns add " + NETNS_NAME).c_str());
  std::system(("sudo ip link add dev " + WG_IF + " type wireguard").c_str());

  const std::string tmpConf = "/tmp/wg-ardc-" + std::to_string(getpid()) + ".conf";
  {
    std::ofstream ofs(tmpConf);
    if (!ofs.is_open())
      return false;
    ofs << "[Interface]\n"
        << "PrivateKey = REDACTED_PRIVATE_KEY=\n\n"
        << "[Peer]\n"
        << "PublicKey = REDACTED_PUBLIC_KEY=\n"
        << "AllowedIPs = 0.0.0.0/0, ::/0\n"
        << "Endpoint = 127.0.0.1:51820\n"
        << "PersistentKeepalive = 25\n";
  }

  std::system(("sudo wg setconf " + WG_IF + " " + tmpConf).c_str());
  fs::remove(tmpConf);

  std::system(("sudo ip link set " + WG_IF + " netns " + NETNS_NAME).c_str());
  std::system(("sudo ip -n " + NETNS_NAME + " addr add 10.2.0.2/32 dev " + WG_IF).c_str());
  std::system(("sudo ip -n " + NETNS_NAME + " link set " + WG_IF + " mtu 1420").c_str());
  std::system(("sudo ip -n " + NETNS_NAME + " link set " + WG_IF + " up").c_str());
  std::system(("sudo ip -n " + NETNS_NAME + " link set lo up").c_str());
  std::system(("sudo ip -n " + NETNS_NAME + " route add default dev " + WG_IF).c_str());

  std::system(("sudo mkdir -p /etc/netns/" + NETNS_NAME).c_str());
  std::system(("printf 'nameserver 10.2.0.1\\nnameserver 1.1.1.1\\nnameserver 8.8.8.8\\n' | sudo tee /etc/netns/" + NETNS_NAME + "/resolv.conf >/dev/null").c_str());

  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  std::string testEgress = runCommand("sudo ip netns exec " + NETNS_NAME + " curl -s -m 5 https://api.ipify.org 2>/dev/null");
  return !testEgress.empty();
}

inline bool launchDiscord(const LaunchTarget &target) {
  const char *home = std::getenv("HOME");
  if (home) {
    const std::vector<std::string> configs = {"discord", "discordcanary", "discordptb", "vesktop"};
    for (const auto &cfg : configs) {
      fs::remove(fs::path(home) / ".config" / cfg / "SingletonLock");
      fs::remove(fs::path(home) / ".config" / cfg / "SingletonCookie");
      fs::remove(fs::path(home) / ".config" / cfg / "SingletonSocket");
    }
  }

  std::string user = runCommand("whoami");
  std::string uid = runCommand("id -u");
  if (user.empty()) user = "alexsanderalri";
  if (uid.empty()) uid = "1000";

  // Detecção dinâmica de ambiente gráfico (X11 / Wayland)
  std::ostringstream envVars;
  envVars << "HOME=" << (home ? home : "/home/" + user) << " "
          << "USER=" << user << " "
          << "LOGNAME=" << user << " "
          << "XDG_RUNTIME_DIR=/run/user/" << uid << " ";

  const char *display = std::getenv("DISPLAY");
  if (display && *display) {
    envVars << "DISPLAY=" << display << " ";
  } else {
    envVars << "DISPLAY=:0 ";
  }

  const char *waylandDisplay = std::getenv("WAYLAND_DISPLAY");
  if (waylandDisplay && *waylandDisplay) {
    envVars << "WAYLAND_DISPLAY=" << waylandDisplay << " ";
  } else {
    // Verificar se existe socket wayland na pasta do usuário
    if (fs::exists("/run/user/" + uid + "/wayland-0")) {
      envVars << "WAYLAND_DISPLAY=wayland-0 ";
    }
  }

  const char *xauth = std::getenv("XAUTHORITY");
  if (xauth && *xauth) {
    envVars << "XAUTHORITY=" << xauth << " ";
  } else if (home && fs::exists(fs::path(home) / ".Xauthority")) {
    envVars << "XAUTHORITY=" << fs::path(home) / ".Xauthority" << " ";
  }

  // Audio e Sessao
  envVars << "DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/" << uid << "/bus "
          << "PULSE_SERVER=unix:/run/user/" << uid << "/pulse/native "
          << "ELECTRON_OZONE_PLATFORM_HINT=auto ";

  const char *desktop = std::getenv("XDG_CURRENT_DESKTOP");
  if (desktop && *desktop) {
    envVars << "XDG_CURRENT_DESKTOP=" << desktop << " ";
  }

  const char *sessionType = std::getenv("XDG_SESSION_TYPE");
  if (sessionType && *sessionType) {
    envVars << "XDG_SESSION_TYPE=" << sessionType << " ";
  }

  std::ostringstream cmd;
  cmd << "sudo ip netns exec " << NETNS_NAME << " setsid -f sudo -u " << user << " env "
      << envVars.str()
      << target.command << " >/tmp/ar-discord-vpn.log 2>&1";

  int ret = std::system(cmd.str().c_str());
  std::this_thread::sleep_for(std::chrono::seconds(2));
  return ret == 0 && isDiscordRunning();
}
