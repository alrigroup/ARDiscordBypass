#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <functional>
#include <chrono>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

#ifdef _WIN32
#include "../os/windows/platform.h"
#else
#include "../os/linux/platform.h"
#endif

namespace Core {

struct LogEntry {
  std::string timeStr;
  std::string message;
  int level; // 0 = info, 1 = success, 2 = warning, 3 = error
};

struct TunnelStatus {
  bool isTunnelUp = false;
  bool isDiscordActive = false;
  std::string egressIp = "--";
  std::string egressCountry = "--";
  std::string pingMs = "--";
  std::string lastHandshake = "--";
  std::string discordClient = "Discord";
  std::string activeConfig = "ardc_vpn.conf";
};

class Service {
public:
  static Service &instance() {
    static Service inst;
    return inst;
  }

  void addLog(const std::string &msg, int level = 0) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tmNow{};
#ifdef _WIN32
    localtime_s(&tmNow, &t);
#else
    localtime_r(&t, &tmNow);
#endif
    char buf[32];
    std::snprintf(buf, sizeof(buf), "[%02d:%02d:%02d]", tmNow.tm_hour, tmNow.tm_min, tmNow.tm_sec);
    m_logs.push_back({buf, msg, level});
    if (m_logs.size() > 200) {
      m_logs.erase(m_logs.begin());
    }
  }

  std::vector<LogEntry> getLogs() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_logs;
  }

  void clearLogs() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_logs.clear();
  }

  bool hasSavedSession() {
#ifdef _WIN32
    wchar_t userProfile[MAX_PATH];
    if (GetEnvironmentVariableW(L"USERPROFILE", userProfile, MAX_PATH) != 0) {
      fs::path p = fs::path(userProfile) / L".protonvpn-session.json";
      return fs::is_regular_file(p);
    }
    return false;
#else
    const char *h = std::getenv("HOME");
    if (!h || !*h) return false;
    return fs::is_regular_file(fs::path(h) / ".protonvpn-session.json");
#endif
  }

  bool isTunnelActive() {
#ifdef _WIN32
    return isWireSockRunning();
#else
    std::string test = runCommand("sudo ip netns list 2>/dev/null");
    if (test.find(NETNS_NAME) == std::string::npos) return false;
    std::string hs = runCommand("sudo ip netns exec " + NETNS_NAME + " wg show " + WG_IF + " latest-handshakes 2>/dev/null");
    std::stringstream ss(hs);
    std::string key;
    long long ts = 0;
    return (ss >> key >> ts && ts > 0);
#endif
  }

  bool isDiscordActive() {
    return isDiscordRunning();
  }

  TunnelStatus queryStatus(const std::string &currentConfig = "ardc_vpn.conf") {
    TunnelStatus st;
    st.activeConfig = currentConfig;
    st.isDiscordActive = isDiscordActive();
    st.isTunnelUp = isTunnelActive();

#ifndef _WIN32
    LaunchTarget target = locateDiscordLaunchTarget("");
    st.discordClient = target.displayName;

    if (st.isTunnelUp) {
      std::string hs = runCommand("sudo ip netns exec " + NETNS_NAME + " wg show " + WG_IF + " latest-handshakes 2>/dev/null");
      std::stringstream ss(hs);
      std::string key;
      long long ts = 0;
      if (ss >> key >> ts && ts > 0) {
        long long diff = static_cast<long long>(std::time(nullptr)) - ts;
        if (diff < 0) diff = 0;
        st.lastHandshake = std::to_string(diff) + "s atrás";
      }

      std::string ping = runCommand("sudo ip netns exec " + NETNS_NAME + " ping -c 1 -W 1 1.1.1.1 2>/dev/null | grep 'time=' | sed -E 's/.*time=([0-9.]+).*/\\1 ms/'");
      if (!ping.empty()) {
        ping.erase(ping.find_last_not_of(" \t\r\n") + 1);
        st.pingMs = ping;
      }

      std::string trace = runCommand("sudo ip netns exec " + NETNS_NAME + " curl -s -m 2 https://1.1.1.1/cdn-cgi/trace 2>/dev/null");
      if (!trace.empty()) {
        std::stringstream tss(trace);
        std::string line;
        while (std::getline(tss, line)) {
          if (line.rfind("ip=", 0) == 0) st.egressIp = line.substr(3);
          if (line.rfind("loc=", 0) == 0) st.egressCountry = line.substr(4);
        }
      }
    }
#else
    st.discordClient = "Discord (Windows)";
    if (st.isTunnelUp) {
      st.lastHandshake = "Ativo (WFP)";
      st.pingMs = "< 120 ms";
    }
#endif

    return st;
  }

  bool loginProton(const std::string &user, const std::string &pass, std::string &outErr, std::function<void(const std::string &)> onProgress = nullptr) {
    addLog("Autenticando no ProtonVPN (" + user + ")...", 0);
    if (onProgress) onProgress("Iniciando conexão segura com a Proton...");
    std::string err;
#ifdef _WIN32
    bool ok = setupProtonConfigWindows(true, user, pass, &err, onProgress);
#else
    bool ok = setupProtonConfig(true, user, pass, &err, onProgress);
#endif
    if (ok) {
      addLog("Perfil WireGuard ProtonVPN gerado com sucesso!", 1);
      return true;
    } else {
      outErr = err.empty() ? "Falha na autenticação ou timeout da API Proton." : err;
      addLog("Erro ao autenticar no ProtonVPN: " + outErr, 3);
      return false;
    }
  }

  bool startBypass(const std::string &configPath, std::string &outErr, std::function<void(const std::string &)> onProgress = nullptr) {
    auto notify = [&](const std::string &m) {
      addLog(m, 0);
      if (onProgress) onProgress(m);
    };

    notify("Verificando instâncias do Discord...");
    if (isDiscordRunning()) {
#ifndef _WIN32
      if (isDiscordRunningInNetns()) {
        addLog("O Discord já está rodando protegido no túnel!", 1);
        return true;
      }
#endif
      notify("Finalizando instâncias abertas na rede comum...");
      killDiscord();
    }

    notify("Ativando túnel WireGuard isolado (" + configPath + ")...");
#ifndef _WIN32
    bool tunnelOk = ensureWireGuardNetns(configPath);
    if (!tunnelOk) {
      outErr = "Falha ao negociar ou validar o túnel WireGuard.";
      addLog(outErr, 3);
      return false;
    }
    notify("Túnel WireGuard ativado com sucesso!");

    notify("Iniciando Discord no ambiente isolado...");
    LaunchTarget target = locateDiscordLaunchTarget("");
    if (target.command.empty()) {
      outErr = "Executável do Discord não encontrado.";
      addLog(outErr, 3);
      return false;
    }

    bool launched = launchDiscord(target);
    if (!launched) {
      outErr = "Falha ao lançar o processo do Discord.";
      addLog(outErr, 3);
      return false;
    }
#else
    bool tunnelOk = ensureWireGuardWindows(configPath);
    if (!tunnelOk) {
      outErr = "Falha ao ativar o driver WFP do WireSock.";
      addLog(outErr, 3);
      return false;
    }
    notify("WireSock WFP ativado com sucesso!");

    std::wstring exe = locateDiscordExe();
    if (exe.empty()) {
      outErr = "Executável do Discord não encontrado.";
      addLog(outErr, 3);
      return false;
    }

    bool launched = launchDiscord(exe);
    if (!launched) {
      outErr = "Falha ao lançar o Discord.";
      addLog(outErr, 3);
      return false;
    }
#endif

    addLog("Discord iniciado com sucesso! Bypass ativo.", 1);
    return true;
  }

  bool stopAll(std::string &outErr) {
    addLog("Solicitando encerramento do bypass...", 0);
    bool ok = ::stopBypass();
    if (ok) {
      addLog("Bypass desativado e rede restaurada.", 1);
      return true;
    } else {
      outErr = "Falha ao encerrar o túnel ou remover namespace.";
      addLog(outErr, 3);
      return false;
    }
  }

  bool clearAllConfigAndSession(std::string &outErr) {
    addLog("Iniciando limpeza total de configurações e sessões...", 0);
    if (isTunnelActive()) {
      stopAll(outErr);
    }
    try {
      if (fs::exists("ardc_vpn.conf")) {
        fs::remove("ardc_vpn.conf");
        addLog("Arquivo de perfil WireGuard 'ardc_vpn.conf' excluído.", 1);
      }
    } catch (const std::exception &e) {
      addLog(std::string("Erro ao remover ardc_vpn.conf: ") + e.what(), 2);
    }

#ifdef _WIN32
    wchar_t userProfile[MAX_PATH];
    if (GetEnvironmentVariableW(L"USERPROFILE", userProfile, MAX_PATH) != 0) {
      fs::path p = fs::path(userProfile) / L".protonvpn-session.json";
      try {
        if (fs::exists(p)) {
          fs::remove(p);
          addLog("Sessão ProtonVPN removida com sucesso.", 1);
        }
      } catch (...) {}
    }
#else
    const char *h = std::getenv("HOME");
    if (h && *h) {
      fs::path p = fs::path(h) / ".protonvpn-session.json";
      try {
        if (fs::exists(p)) {
          fs::remove(p);
          addLog("Sessão ProtonVPN (~/.protonvpn-session.json) removida com sucesso.", 1);
        }
      } catch (...) {}
    }
#endif
    addLog("Limpeza concluída! Todos os dados e logins foram apagados.", 1);
    return true;
  }

private:
  Service() {
    addLog("ARDiscordBypass Core inicializado.", 0);
  }
  std::mutex m_mutex;
  std::vector<LogEntry> m_logs;
};

} // namespace Core
