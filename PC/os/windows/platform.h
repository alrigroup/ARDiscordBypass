#pragma once

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <winhttp.h>
#include <tlhelp32.h>
#include <shellapi.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace fs = std::filesystem;

inline const std::string WG_CONFIG_NAME = "ardiscord-vpn.conf";

inline std::string runCommand(const std::string &cmd) {
  std::string result;
  FILE *pipe = _popen(cmd.c_str(), "r");
  if (!pipe)
    return "";
  char buffer[256];
  while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
    result += buffer;
  _pclose(pipe);
  while (!result.empty() && (result.back() == '\n' || result.back() == '\r'))
    result.pop_back();
  return result;
}

inline bool isDiscordRunning() {
  HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (hSnap == INVALID_HANDLE_VALUE)
    return false;

  PROCESSENTRY32W pe;
  pe.dwSize = sizeof(pe);

  bool running = false;
  if (Process32FirstW(hSnap, &pe)) {
    do {
      std::wstring name(pe.szExeFile);
      if (_wcsicmp(pe.szExeFile, L"Discord.exe") == 0 ||
          _wcsicmp(pe.szExeFile, L"DiscordCanary.exe") == 0 ||
          _wcsicmp(pe.szExeFile, L"DiscordPTB.exe") == 0 ||
          _wcsicmp(pe.szExeFile, L"Vesktop.exe") == 0) {
        running = true;
        break;
      }
    } while (Process32NextW(hSnap, &pe));
  }

  CloseHandle(hSnap);
  return running;
}

inline void killDiscord() {
  // Encerramento suave sem /F inicialmente
  _wsystem(L"taskkill /IM Discord.exe >nul 2>&1");
  _wsystem(L"taskkill /IM DiscordCanary.exe >nul 2>&1");
  _wsystem(L"taskkill /IM DiscordPTB.exe >nul 2>&1");
  _wsystem(L"taskkill /IM Vesktop.exe >nul 2>&1");

  for (int i = 0; i < 10; ++i) {
    if (!isDiscordRunning())
      return;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }

  // Fallback caso algum processo zumbi tenha ficado preso
  if (isDiscordRunning()) {
    _wsystem(L"taskkill /F /IM Discord.exe >nul 2>&1");
    _wsystem(L"taskkill /F /IM DiscordCanary.exe >nul 2>&1");
    _wsystem(L"taskkill /F /IM DiscordPTB.exe >nul 2>&1");
  }
}

inline std::wstring findDiscordExeIn(const std::wstring &basePath) {
  std::wstring discordRoot = basePath + L"\\Discord";
  if (!fs::exists(discordRoot))
    return L"";

  std::wstring latestPath;
  std::wstring latestVersion;
  try {
    for (const auto &entry : fs::directory_iterator(discordRoot)) {
      if (!entry.is_directory())
        continue;
      std::wstring folder = entry.path().filename().wstring();
      if (folder.rfind(L"app-", 0) != 0)
        continue;
      std::wstring exePath = (entry.path() / L"Discord.exe").wstring();
      if (fs::exists(exePath)) {
        if (latestVersion.empty() || folder > latestVersion) {
          latestVersion = folder;
          latestPath = exePath;
        }
      }
    }
  } catch (...) {}
  return latestPath;
}

inline std::wstring locateDiscordExe() {
  wchar_t pathBuf[MAX_PATH];
  if (GetEnvironmentVariableW(L"LOCALAPPDATA", pathBuf, MAX_PATH) != 0) {
    std::wstring result = findDiscordExeIn(std::wstring(pathBuf));
    if (!result.empty())
      return result;
  }
  if (GetEnvironmentVariableW(L"ProgramFiles", pathBuf, MAX_PATH) != 0) {
    std::wstring result = findDiscordExeIn(std::wstring(pathBuf));
    if (!result.empty())
      return result;
  }
  if (GetEnvironmentVariableW(L"ProgramFiles(x86)", pathBuf, MAX_PATH) != 0) {
    std::wstring result = findDiscordExeIn(std::wstring(pathBuf));
    if (!result.empty())
      return result;
  }
  return L"";
}

inline std::wstring findWireSockExecutable() {
  // 1. Verificar pasta do próprio executável
  wchar_t modulePath[MAX_PATH];
  if (GetModuleFileNameW(nullptr, modulePath, MAX_PATH) != 0) {
    fs::path dir = fs::path(modulePath).parent_path();
    if (fs::exists(dir / L"wiresock-client.exe"))
      return (dir / L"wiresock-client.exe").wstring();
    if (fs::exists(dir / L"bin" / L"wiresock-client.exe"))
      return (dir / L"bin" / L"wiresock-client.exe").wstring();
  }

  // 2. Verificar Program Files
  const std::vector<std::wstring> candidates = {
      L"C:\\Program Files\\WireSock Secure Connect\\wiresock-client.exe",
      L"C:\\Program Files\\WireSock Secure Connect\\sdk\\wiresock-client.exe",
      L"C:\\Program Files (x86)\\WireSock Secure Connect\\wiresock-client.exe",
      L"C:\\Program Files (x86)\\WireSock Secure Connect\\sdk\\wiresock-client.exe"
  };

  for (const auto &cand : candidates) {
    if (fs::exists(cand))
      return cand;
  }

  // 3. Verificar via PATH
  std::string whereOut = runCommand("where wiresock-client.exe 2>nul");
  if (!whereOut.empty()) {
    std::wstring wWhere(whereOut.begin(), whereOut.end());
    if (fs::exists(wWhere))
      return wWhere;
  }

  return L"";
}

inline bool ensureWireSockInstalled() {
  std::wstring wsExe = findWireSockExecutable();
  if (!wsExe.empty())
    return true;

  std::cout << std::endl;
  std::cout << "\033[36m[INFO]\033[0m WireSock WFP Driver não encontrado no seu Windows." << std::endl;
  std::cout << "\033[36m[INFO]\033[0m Baixando e configurando automaticamente a dependência para o programa funcionar..." << std::endl;
  std::cout << "\033[33m[AVISO]\033[0m Caso o Windows solicite permissão de Administrador (UAC), clique em \"Sim\" para autorizar o driver de rede." << std::endl;

  // Tenta instalar via winget silenciosamente primeiro
  int wingetRet = _wsystem(L"powershell -NoProfile -ExecutionPolicy Bypass -Command "
                           L"\"Start-Process winget -ArgumentList 'install -e --id WireSock.WireSockSecureConnectSDK --silent --accept-source-agreements --accept-package-agreements' -Wait -Verb RunAs\" >nul 2>&1");

  if (wingetRet == 0 && !findWireSockExecutable().empty()) {
    std::cout << "\033[32m[ ✔ ] Dependência WireSock instalada com sucesso!\033[0m" << std::endl << std::endl;
    return true;
  }

  // Fallback: download direto do instalador oficial do WireSock SDK
  std::wstring tempInstaller = fs::temp_directory_path() / L"wiresock-sdk.exe";
  std::wstring dlCmd = L"powershell -NoProfile -ExecutionPolicy Bypass -Command \""
                       L"Invoke-WebRequest -Uri 'https://wiresock.net/_api/download-release.php?product=wiresock-secure-connect-sdk&platform=x64&version=3.4.8.1&channel=winget' -OutFile '" +
                       tempInstaller + L"'; "
                       L"Start-Process -FilePath '" + tempInstaller + L"' -ArgumentList @('/quiet','/norestart') -Wait -Verb RunAs\"";

  _wsystem(dlCmd.c_str());

  bool ok = !findWireSockExecutable().empty();
  if (ok) {
    std::cout << "\033[32m[ ✔ ] Dependência WireSock instalada e configurada com sucesso!\033[0m" << std::endl << std::endl;
  }
  return ok;
}

inline bool isWireSockRunning() {
  HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (hSnap == INVALID_HANDLE_VALUE)
    return false;

  PROCESSENTRY32W pe;
  pe.dwSize = sizeof(pe);

  bool running = false;
  if (Process32FirstW(hSnap, &pe)) {
    do {
      if (_wcsicmp(pe.szExeFile, L"wiresock-client.exe") == 0) {
        running = true;
        break;
      }
    } while (Process32NextW(hSnap, &pe));
  }

  CloseHandle(hSnap);
  return running;
}

inline bool ensureWireGuardWindows() {
  if (!ensureWireSockInstalled()) {
    std::cerr << "[ERRO] Não foi possível encontrar ou instalar o WireSock no Windows." << std::endl;
    return false;
  }

  std::wstring wsExe = findWireSockExecutable();
  if (wsExe.empty())
    return false;

  // Se o WireSock já estiver rodando, ele já está interceptando
  if (isWireSockRunning())
    return true;

  // Criar arquivo de configuração com AllowedApps para isolar apenas o Discord
  fs::path confPath = fs::temp_directory_path() / WG_CONFIG_NAME;
  std::ofstream ofs(confPath);
  if (!ofs.is_open())
    return false;

  ofs << "[Interface]\n"
      << "PrivateKey = sLPBSsrhzoqZSOY/XxAzGAy5F+sQKQIIE3WoxG8buWM=\n"
      << "Address = 10.2.0.2/32\n"
      << "\n[Peer]\n"
      << "PublicKey = mkI+cC9ggzfMdZy1cl3Fl01gPJJxsLXjshXAN8EedQ8=\n"
      << "AllowedIPs = 0.0.0.0/0, ::/0\n"
      << "Endpoint = 84.20.27.53:51820\n"
      << "PersistentKeepalive = 25\n"
      << "#@ws:AllowedApps = Discord, Update, DiscordCanary, DiscordPTB, Vesktop\n";
  ofs.close();

  // Iniciar WireSock em background
  std::wstring cmd = L"\"" + wsExe + L"\" run -config \"" + confPath.wstring() + L"\" -log-level none";

  STARTUPINFOW si = {sizeof(si)};
  si.dwFlags = STARTF_USESHOWWINDOW;
  si.wShowWindow = SW_HIDE;
  PROCESS_INFORMATION pi = {0};

  BOOL ok = CreateProcessW(
      nullptr, const_cast<LPWSTR>(cmd.data()), nullptr, nullptr, FALSE,
      CREATE_NO_WINDOW | DETACHED_PROCESS, nullptr, nullptr, &si, &pi);

  if (ok) {
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return isWireSockRunning();
  }

  return false;
}

inline bool launchDiscord(const std::wstring &exePath) {
  // Limpar travas Singleton no Windows
  wchar_t appData[MAX_PATH];
  if (GetEnvironmentVariableW(L"APPDATA", appData, MAX_PATH) != 0) {
    fs::path base(appData);
    const std::vector<std::wstring> cfgs = {L"discord", L"discordcanary", L"discordptb"};
    for (const auto &cfg : cfgs) {
      std::error_code ec;
      fs::remove(base / cfg / L"SingletonLock", ec);
      fs::remove(base / cfg / L"SingletonCookie", ec);
      fs::remove(base / cfg / L"SingletonSocket", ec);
    }
  }

  std::wstring cmdLine = L"\"" + exePath + L"\"";

  SECURITY_ATTRIBUTES sa = {sizeof(sa), nullptr, TRUE};
  HANDLE hNull = CreateFileW(
      L"NUL", GENERIC_WRITE | GENERIC_READ,
      FILE_SHARE_WRITE | FILE_SHARE_READ, &sa, OPEN_EXISTING,
      FILE_ATTRIBUTE_NORMAL, nullptr);

  STARTUPINFOW si = {sizeof(si)};
  si.dwFlags = STARTF_USESTDHANDLES;
  si.hStdOutput = hNull;
  si.hStdError = hNull;
  si.hStdInput = hNull;

  PROCESS_INFORMATION pi = {0};
  std::wstring workDir = fs::path(exePath).parent_path().wstring();

  BOOL ok = CreateProcessW(
      nullptr, const_cast<LPWSTR>(cmdLine.data()), nullptr, nullptr, TRUE,
      CREATE_NO_WINDOW | DETACHED_PROCESS | CREATE_NEW_PROCESS_GROUP |
          CREATE_BREAKAWAY_FROM_JOB,
      nullptr, workDir.c_str(), &si, &pi);

  if (hNull != INVALID_HANDLE_VALUE)
    CloseHandle(hNull);

  if (ok) {
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return isDiscordRunning();
  }
  return false;
}
