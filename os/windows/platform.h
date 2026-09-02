#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <winhttp.h>
#include <tlhelp32.h>
#include <shellapi.h>

#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "winhttp.lib")

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace fs = std::filesystem;

inline bool isDiscordRunning(
    const std::wstring &targetExe = L"Discord.exe") {
  HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (hSnap == INVALID_HANDLE_VALUE)
    return false;

  PROCESSENTRY32W pe;
  pe.dwSize = sizeof(pe);

  if (Process32FirstW(hSnap, &pe)) {
    do {
      if (_wcsicmp(pe.szExeFile, targetExe.c_str()) == 0) {
        CloseHandle(hSnap);
        return true;
      }
    } while (Process32NextW(hSnap, &pe));
  }

  CloseHandle(hSnap);
  return false;
}

inline void killDiscord(
    const std::wstring &targetExe = L"Discord.exe") {
  std::wstring cmd =
      L"taskkill /F /T /IM \"" + targetExe + L"\" >nul 2>&1";
  _wsystem(cmd.c_str());
  std::this_thread::sleep_for(std::chrono::seconds(1));
}

inline std::wstring findDiscordExeIn(const std::wstring &basePath) {
  std::wstring discordRoot = basePath + L"\\Discord";
  if (!fs::exists(discordRoot))
    return L"";

  std::wstring latestPath;
  std::wstring latestVersion;
  for (const auto &entry : fs::directory_iterator(discordRoot)) {
    if (!entry.is_directory())
      continue;
    std::wstring folder = entry.path().filename().wstring();
    if (folder.rfind(L"app-", 0) != 0)
      continue;
    std::wstring exePath = entry.path() / L"Discord.exe";
    if (fs::exists(exePath)) {
      if (latestVersion.empty() || folder > latestVersion) {
        latestVersion = folder;
        latestPath = exePath;
      }
    }
  }
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

inline bool launchDiscord(const std::wstring &exePath,
                           const std::string &proxyEndpoint) {
  std::wstring cmdLine = L"\"" + exePath + L"\"";

  if (!proxyEndpoint.empty()) {
    std::wstring wProxy(proxyEndpoint.begin(), proxyEndpoint.end());
    cmdLine += L" --proxy-server=" + wProxy +
               L" --proxy-bypass-list=\"cdn.discordapp.com;*.discordapp.net;*.discord."
               L"media;<local>\"";
  }

  SECURITY_ATTRIBUTES sa = {sizeof(sa), nullptr, TRUE};
  HANDLE hNull =
      CreateFileW(L"NUL", GENERIC_WRITE | GENERIC_READ,
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
    return true;
  }
  return false;
}

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

inline bool hasCurl() {
  FILE *pipe = _popen("where curl >nul 2>&1", "r");
  if (!pipe)
    return false;
  int ret = _pclose(pipe);
  return ret == 0;
}

inline std::vector<std::string> fetchProxies() {
  std::vector<std::string> proxies;
  std::string data;

  std::string url =
      "https://api.proxyscrape.com/v4/free-proxy-list/get?"
      "request=display_proxies&proxy_format=protocolipport"
      "&format=text&protocol=socks5&timeout=5000";

  if (hasCurl()) {
    data = runCommand("curl -s --max-time 10 \"" + url + "\"");
  } else {
    std::string psCmd =
        "powershell -NoProfile -Command \""
        "Invoke-WebRequest -Uri '" + url + "' -UseBasicParsing | Select-Object -ExpandProperty Content"
        "\"";
    data = runCommand(psCmd);
  }
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
  std::string result;
  if (hasCurl()) {
    result = runCommand("curl -s --max-time 8 --socks5-hostname " + proxy +
                         " https://discord.com/api/v10/gateway 2>nul");
  } else {
    std::string psCmd =
        "powershell -NoProfile -Command \""
        "$proxy = [System.Net.WebProxy]::new('" + proxy + "');"
        "$wc = New-Object System.Net.WebClient; $wc.Proxy = $proxy;"
        "try { $wc.DownloadString('https://discord.com/api/v10/gateway') } catch { '' }"
        "\"";
    result = runCommand(psCmd);
  }
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
