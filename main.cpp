#ifdef _WIN32
#include "os/windows/platform.h"
#else
#include "os/linux/platform.h"
#endif

#include <regex>

#define APP_VERSION "2.0.0"
#define GITHUB_REPO "alrigroup/ARDiscordBypass"
#define GITHUB_API_URL "https://api.github.com/repos/" GITHUB_REPO "/releases/latest"

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
  std::cout << "              ALRI GROUP - ARDiscordBypass v" << APP_VERSION << " (Discord Live)" << std::endl;
  std::cout << "          Esse projeto é open source em https://github.com/" << GITHUB_REPO << std::endl;
  std::cout << "=========================================================================================" << std::endl << std::endl;
}

std::string fetchLatestVersion() {
#ifdef _WIN32
  HINTERNET hSession = WinHttpOpen(L"ARDCB/2.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                    WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
  if (!hSession)
    return "";

  URL_COMPONENTS urlComp = {0};
  urlComp.dwStructSize = sizeof(urlComp);
  urlComp.lpszHostName = const_cast<LPWSTR>(L"api.github.com");
  urlComp.dwHostNameLength = (DWORD)wcslen(L"api.github.com");
  urlComp.lpszUrlPath = const_cast<LPWSTR>(L"/repos/" GITHUB_REPO L"/releases/latest");
  urlComp.dwUrlPathLength = (DWORD)wcslen(L"/repos/" GITHUB_REPO L"/releases/latest");
  urlComp.nScheme = INTERNET_SCHEME_HTTPS;

  HINTERNET hConnect = WinHttpConnect(hSession, urlComp.lpszHostName,
                                       urlComp.nScheme == INTERNET_SCHEME_HTTPS
                                           ? INTERNET_DEFAULT_HTTPS_PORT
                                           : INTERNET_DEFAULT_HTTP_PORT, 0);
  if (!hConnect) {
    WinHttpCloseHandle(hSession);
    return "";
  }

  HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", urlComp.lpszUrlPath,
                                           nullptr, WINHTTP_NO_REFERER,
                                           WINHTTP_DEFAULT_ACCEPT_TYPES,
                                           WINHTTP_FLAG_SECURE);
  if (!hRequest) {
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return "";
  }

  WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                      WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
  WinHttpReceiveResponse(hRequest, nullptr);

  std::string response;
  DWORD bytesRead = 0;
  char buffer[4096];
  while (WinHttpReadData(hRequest, buffer, sizeof(buffer) - 1, &bytesRead) &&
         bytesRead > 0) {
    buffer[bytesRead] = '\0';
    response += buffer;
    bytesRead = 0;
  }

  WinHttpCloseHandle(hRequest);
  WinHttpCloseHandle(hConnect);
  WinHttpCloseHandle(hSession);
#else
  std::string response = runCommand(
      "curl -s --max-time 10 "
      "\"https://api.github.com/repos/" GITHUB_REPO "/releases/latest\"");
#endif

  std::regex tagRegex("\"tag_name\"\\s*:\\s*\"([^\"]+)\"");
  std::smatch match;
  if (std::regex_search(response, match, tagRegex)) {
    std::string tag = match[1].str();
    if (!tag.empty() && tag[0] == 'v')
      tag = tag.substr(1);
    return tag;
  }
  return "";
}

bool checkForUpdate() {
  logf("Verificando atualizações...");
  std::string latest = fetchLatestVersion();
  if (latest.empty()) {
    logf("Não foi possível verificar atualizações.");
    return false;
  }

  if (latest != APP_VERSION) {
    logf("NOVA VERSÃO DISPONÍVEL: " + latest + " (atual: " + APP_VERSION + ")");
    logf("Baixe em: https://github.com/" GITHUB_REPO "/releases/latest");
    return true;
  }

  logf("Você está na versão mais recente (v" APP_VERSION ").");
  return false;
}

int main(int argc, char *argv[]) {
#ifdef _WIN32
  SetConsoleOutputCP(CP_UTF8);
#endif

  printBanner();

#ifdef _WIN32
  WSADATA wsaData;
  WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

  checkForUpdate();
  std::cout << std::endl;

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

  std::string selectedProxy = findWorkingProxy();
  bool hasProxy = !selectedProxy.empty();

  if (hasProxy) {
    logf("SERVIDOR SELECIONADO: " + selectedProxy);
  } else {
    logf("AVISO: Nenhum proxy funcional encontrado.");
    logf("O Discord será aberto sem bypass de proxy.");
  }

  logf("INICIANDO DISCORD...");
  if (launchDiscord(discordExe, hasProxy ? selectedProxy : "")) {
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
