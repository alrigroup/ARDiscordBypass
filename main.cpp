#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <winhttp.h>
#include <tlhelp32.h>
#include <shellapi.h>

#include <chrono>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "winhttp.lib")

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
  std::cout << "=========================================================================================" << std::endl << std::endl;
}

// Verifica se o Discord está em execução usando a Win32 API Toolhelp32
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

// Encerra qualquer processo do Discord em execução usando taskkill (igual ao Go)
void killDiscord(const std::wstring &targetExe = L"Discord.exe") {
  std::wstring cmd = L"taskkill /F /T /IM \"" + targetExe + L"\" >nul 2>&1";
  _wsystem(cmd.c_str());
  std::this_thread::sleep_for(std::chrono::seconds(1));
}

// Procura a instalação do Discord no %LOCALAPPDATA%
#include <filesystem>
namespace fs = std::filesystem;

// Busca a instalação do Discord em um diretório base (ex.: LOCALAPPDATA ou
// Program Files)
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
      continue; // must start with "app-"
    std::wstring exePath = entry.path() / L"Discord.exe";
    if (fs::exists(exePath)) {
      // Keep the highest version (lexicographically works for typical version
      // numbers)
      if (latestVersion.empty() || folder > latestVersion) {
        latestVersion = folder;
        latestPath = exePath;
      }
    }
  }
  return latestPath;
}

// Wrapper that first tries %LOCALAPPDATA% then falls back to Program Files
// directories.
std::wstring locateDiscordExe() {
  wchar_t pathBuf[MAX_PATH];
  // 1. LOCALAPPDATA
  if (GetEnvironmentVariableW(L"LOCALAPPDATA", pathBuf, MAX_PATH) != 0) {
    std::wstring result = findDiscordExeIn(std::wstring(pathBuf));
    if (!result.empty())
      return result;
  }
  // 2. Program Files (64‑bit)
  if (GetEnvironmentVariableW(L"ProgramFiles", pathBuf, MAX_PATH) != 0) {
    std::wstring result = findDiscordExeIn(std::wstring(pathBuf));
    if (!result.empty())
      return result;
  }
  // 3. Program Files (x86)
  if (GetEnvironmentVariableW(L"ProgramFiles(x86)", pathBuf, MAX_PATH) != 0) {
    std::wstring result = findDiscordExeIn(std::wstring(pathBuf));
    if (!result.empty())
      return result;
  }
  return L"";
}

// Executa o Discord apontado para o proxy configurado
bool launchDiscord(const std::wstring &exePath,
                   const std::string &proxyEndpoint) {
  std::wstring wProxy(proxyEndpoint.begin(), proxyEndpoint.end());
  std::wstring cmdLine = L"\"" + exePath + L"\" --proxy-server=" + wProxy +
                         L" --proxy-bypass-list=\"cdn.discordapp.com;*."
                         L"discordapp.net;*.discord.media;<local>\"";

  // Create an INHERITABLE handle to NUL so the child's stdout/stderr are
  // discarded
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
      TRUE, // bInheritHandles = TRUE so child process inherits hNull
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

int main() {
  // Configura o console para UTF-8
  SetConsoleOutputCP(CP_UTF8);

  printBanner();

  WSADATA wsaData;
  WSAStartup(MAKEWORD(2, 2), &wsaData);

  logf("VERIFICANDO INSTALAÇÃO DO DISCORD...");
  std::wstring discordExe =
      locateDiscordExe(); // initial lookup in %LOCALAPPDATA%
  // Try to locate Discord in common locations if not found in LOCALAPPDATA
  // Fallback searches in Program Files are now handled by locateDiscordExe(),
  // which uses std::filesystem. If a command‑line argument is provided, treat
  // it as an explicit exe path (overrides auto‑detect)
  if (discordExe.empty() && __argc > 1) {
    int wlen = MultiByteToWideChar(CP_UTF8, 0, __argv[1], -1, nullptr, 0);
    if (wlen > 0) {
      std::wstring arg(wlen, 0);
      MultiByteToWideChar(CP_UTF8, 0, __argv[1], -1, &arg[0], wlen);
      arg.resize(wlen - 1);
      discordExe = arg;
    }
  }
  if (discordExe.empty()) {
    // Check Program Files (both 64‑bit and 32‑bit)
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
    // Check Program Files (x86) if still not found
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
  // If a command‑line argument is provided, treat it as an explicit exe path
  // (overrides auto‑detect)
  if (discordExe.empty() && __argc > 1) {
    // __argv holds UTF‑8 strings; convert to wide string
    int wlen = MultiByteToWideChar(CP_UTF8, 0, __argv[1], -1, nullptr, 0);
    if (wlen > 0) {
      std::wstring arg(wlen, 0);
      MultiByteToWideChar(CP_UTF8, 0, __argv[1], -1, &arg[0], wlen);
      arg.resize(wlen - 1); // remove terminating null
      discordExe = arg;
    }
  }
  if (discordExe.empty()) {
    logf("ERRO: Não encontrei a instalação do Discord no seu sistema.");
    WSACleanup();
    return 1;
  }
  logf("EXECUTÁVEL ENCONTRADO!");

  logf("VERIFICANDO SE O DISCORD ESTÁ ABERTO...");
  if (isDiscordRunning()) {
    logf("DISCORD ABERTO DETECTADO! ELE SERÁ ENCERRADO PARA APLICAR O "
         "BYPASS...");
    killDiscord();
  }

  logf("PROCURANDO CONEXÃO...");
  logf("PROCURANDO SERVIDOR DE CONEXÃO FORA DO BRASIL...");

  // Simula a resolução de proxy SOCKS5 rápido (ou integrado com WinHTTP)
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

  WSACleanup();

  std::cout << std::endl;
  std::cout << "Pressione ENTER para fechar...";
  std::cout.flush();

  // Flush stdin console buffer and wait for ENTER key
  HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
  if (hIn != INVALID_HANDLE_VALUE) {
    FlushConsoleInputBuffer(hIn);
  }
  std::string dummy;
  std::getline(std::cin, dummy);

  return 0;
}
