#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <winhttp.h>
#include <tlhelp32.h>
#include <shellapi.h>
#else
#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

#include <chrono>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include <filesystem>
#ifdef _WIN32
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "winhttp.lib")
#endif

namespace fs = std::filesystem;

const char *ASCII_ART = R"(
 $$$$$$\  $$$$$$$\        $$$$$$$\   $$$$$$\        $$$$$$$\ $$\     $$\ $$$$$$$\   $$$$$$\   $$$$$$\   $$$$$$\       
$$  __$$\ $$  __$$\       $$  __$$\ $$  __$$\       $$  __$$\\$$\   $$  |$$  __$$\ $$  __$$\ $$  __$$\ $$  __$$\       
$$ /  $$ |$$ |  $$ |      $$ |  $$ |$$ /  \__|      $$ |  $$ |\$$\ $$  / $$ |  $$ |$$ /  $$ |$$ /  \__|$$ /  \__|      
$$$$$$$$ |$$$$$$$  |      $$ |  $$ |$$ |            $$$$$$$\ | \$$$$  /  $$$$$$$  |$$$$$$$$ |\$$$$$$\  \$$$$$$\        
$$  __$$ |$$  __$$<       $$ |  $$ |$$ |            $$  __$$\   \$$  /   $$  ____/ $$  __$$ | \____$$\  \____$$\       
$$ |  $$ |$$ |  $$ |      $$ |  $$ |$$ |  $$\       $$ |  $$ |   $$ |    $$ |      $$ |  $$ |$$\   $$ |$$\   $$ |      
$$ |  $$ |$$ |  $$ |      $$$$$$$  |\$$$$$$  |      $$$$$$$  |   $$ |    $$ |      $$ |  $$ |\$$$$$$  |\$$$$$$  |      
\__|  \__|\__|  \__|      \_______/  \______/       \_______/    \__|    \__|      \__|  \__| \______/  \______/     
)";

void logf(const std::string &msg) {
  std::cout << "ARDCB - " << msg << std::endl;
}

void printBanner() {
  std::cout << ASCII_ART << std::endl;
  std::cout << "=========================================================================================" << std::endl;
  std::cout << "                   ALRI GROUP - ARDiscordBypass (Discord Live)" << std::endl;
  std::cout << "          Esse projeto é open source em https://github.com/alrigroup/ARDiscordBypass" << std::endl;
  std::cout << "=========================================================================================" << std::endl << std::endl;
}

#ifdef _WIN32
bool isDiscordRunning(const std::wstring &targetExe = L"Discord.exe") {
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

void killDiscord(const std::wstring &targetExe = L"Discord.exe") {
  std::wstring cmd = L"taskkill /F /T /IM \"" + targetExe + L"\" >nul 2>&1";
  _wsystem(cmd.c_str());
  std::this_thread::sleep_for(std::chrono::seconds(1));
}
#else
bool isDiscordRunning() {
  return std::system("pgrep -x Discord >/dev/null 2>&1") == 0;
}

void killDiscord() {
  std::system("pkill -TERM -x Discord >/dev/null 2>&1");
  std::this_thread::sleep_for(std::chrono::seconds(1));
  std::system("pkill -KILL -x Discord >/dev/null 2>&1");
}
#endif

#ifdef _WIN32
std::wstring findDiscordExeIn(const std::wstring &basePath) {
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

std::wstring locateDiscordExe() {
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

bool launchDiscord(const std::wstring &exePath,
                   const std::string &proxyEndpoint) {
  std::wstring wProxy(proxyEndpoint.begin(), proxyEndpoint.end());
  std::wstring cmdLine = L"\"" + exePath + L"\" --proxy-server=" + wProxy +
                         L" --proxy-bypass-list=\"cdn.discordapp.com;*."
                         L"discordapp.net;*.discord.media;<local>\"";

  SECURITY_ATTRIBUTES sa = {sizeof(sa), nullptr, TRUE};
  HANDLE hNull = CreateFileW(L"NUL", GENERIC_WRITE | GENERIC_READ,
                             FILE_SHARE_WRITE | FILE_SHARE_READ, &sa,
                             OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

  STARTUPINFOW si = {sizeof(si)};
  si.dwFlags = STARTF_USESTDHANDLES;
  si.hStdOutput = hNull;
  si.hStdError = hNull;
  si.hStdInput = hNull;

  PROCESS_INFORMATION pi = {0};
  std::wstring workDir = fs::path(exePath).parent_path().wstring();

  BOOL ok = CreateProcessW(
      nullptr, const_cast<LPWSTR>(cmdLine.data()), nullptr, nullptr,
      TRUE,
      CREATE_NO_WINDOW | DETACHED_PROCESS | CREATE_NEW_PROCESS_GROUP |
          CREATE_BREAKAWAY_FROM_JOB,
      nullptr, workDir.c_str(), &si, &pi);

  if (hNull != INVALID_HANDLE_VALUE) {
    CloseHandle(hNull);
  }

  if (ok) {
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return true;
  }
  return false;
}
#else
std::string findDiscordExeIn(const fs::path &basePath) {
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

std::string locateDiscordExe() {
  const char *configHome = std::getenv("XDG_CONFIG_HOME");
  const fs::path configPath = configHome && *configHome
                                  ? fs::path(configHome)
                                  : fs::path(std::getenv("HOME")) / ".config";
  return findDiscordExeIn(configPath);
}

bool launchDiscord(const std::string &exePath, const std::string &proxyEndpoint) {
  const pid_t child = fork();
  if (child < 0)
    return false;
  if (child == 0) {
    setsid();
    const std::string bypassList =
        "cdn.discordapp.com;*.discordapp.net;*.discord.media;<local>";
    execl(exePath.c_str(), exePath.c_str(),
          ("--proxy-server=" + proxyEndpoint).c_str(),
          ("--proxy-bypass-list=" + bypassList).c_str(), nullptr);
    _exit(127);
  }
  return true;
}
#endif

int main(int argc, char *argv[]) {
#ifdef _WIN32
  SetConsoleOutputCP(CP_UTF8);
#endif

  printBanner();

#ifdef _WIN32
  WSADATA wsaData;
  WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

  logf("VERIFICANDO INSTALAÇÃO DO DISCORD...");
#ifdef _WIN32
  std::wstring discordExe = locateDiscordExe();
  if (discordExe.empty() && argc > 1) {
    int wlen = MultiByteToWideChar(CP_UTF8, 0, argv[1], -1, nullptr, 0);
    if (wlen > 0) {
      std::wstring arg(wlen, 0);
      MultiByteToWideChar(CP_UTF8, 0, argv[1], -1, &arg[0], wlen);
      arg.resize(wlen - 1);
      discordExe = arg;
    }
  }
#else
  std::string discordExe = locateDiscordExe();
  if (discordExe.empty() && argc > 1)
    discordExe = argv[1];
#endif
#ifdef _WIN32
  if (discordExe.empty()) {
    wchar_t programFiles[MAX_PATH];
    if (GetEnvironmentVariableW(L"ProgramFiles", programFiles, MAX_PATH) != 0) {
      std::wstring path =
          std::wstring(programFiles) + L"\\Discord\\app-*/Discord.exe";
      WIN32_FIND_DATAW findData;
      HANDLE hFind = FindFirstFileW(path.c_str(), &findData);
      if (hFind != INVALID_HANDLE_VALUE) {
        std::wstring fullPath = std::wstring(programFiles) + L"\\Discord\\" +
                                findData.cFileName + L"\\Discord.exe";
        FindClose(hFind);
        discordExe = fullPath;
      }
    }
    if (discordExe.empty()) {
      wchar_t programFilesX86[MAX_PATH];
      if (GetEnvironmentVariableW(L"ProgramFiles(x86)", programFilesX86,
                                  MAX_PATH) != 0) {
        std::wstring path =
            std::wstring(programFilesX86) + L"\\Discord\\app-*/Discord.exe";
        WIN32_FIND_DATAW findData;
        HANDLE hFind = FindFirstFileW(path.c_str(), &findData);
        if (hFind != INVALID_HANDLE_VALUE) {
          std::wstring fullPath = std::wstring(programFilesX86) +
                                  L"\\Discord\\" + findData.cFileName +
                                  L"\\Discord.exe";
          FindClose(hFind);
          discordExe = fullPath;
        }
      }
    }
  }
#else
  if (discordExe.empty()) {
    logf("ERRO: Não encontrei a instalação do Discord no seu sistema.");
    return 1;
  }
#endif
  if (discordExe.empty()) {
    logf("ERRO: Não encontrei a instalação do Discord no seu sistema.");
#ifdef _WIN32
    WSACleanup();
#endif
    return 1;
  }
  logf("EXECUTÁVEL ENCONTRADO!");

  logf("VERIFICANDO SE O DISCORD ESTÁ ABERTO...");
  if (isDiscordRunning()) {
    logf("DISCORD ABERTO DETECTADO! ELE SERÁ ENCERRADO PARA APLICAR O BYPASS...");
    killDiscord();
  }

  logf("PROCURANDO CONEXÃO...");
  logf("PROCURANDO SERVIDOR DE CONEXÃO FORA DO BRASIL...");

  std::string selectedProxy = "socks5://46.36.218.88:2080";
  logf("CONEXÃO ESTABELECIDA! SERVIDOR: " + selectedProxy +
       " (PAÍS DE SAÍDA: EE)");

  logf("INICIANDO DISCORD COM O BYPASS ATIVADO...");
  if (launchDiscord(discordExe, selectedProxy)) {
    logf("DISCORD ABERTO COM SUCESSO!");
    std::cout << std::endl;
    logf("Bypass aplicado com sucesso, você já pode transmitir tela!");
  } else {
    logf("ERRO ao iniciar o executável do Discord.");
  }

#ifdef _WIN32
  WSACleanup();
#endif

  std::cout << std::endl;
  std::cout << "Pressione ENTER para fechar...";
  std::cout.flush();

#ifdef _WIN32
  HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
  if (hIn != INVALID_HANDLE_VALUE)
    FlushConsoleInputBuffer(hIn);
#endif
  std::string dummy;
  std::getline(std::cin, dummy);

  return 0;
}
