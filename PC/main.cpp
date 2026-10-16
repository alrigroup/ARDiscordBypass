#ifdef _WIN32
#include "os/windows/platform.h"
#else
#include "os/linux/platform.h"
#endif

#include "ui.h"

#include <regex>

#define APP_VERSION "3.0.0"
#define GITHUB_REPO "alrigroup/ARDiscordBypass"
#define GITHUB_API_URL "https://api.github.com/repos/" GITHUB_REPO "/releases/latest"

const char *ALRI_LOGO_FULL =
"                    @@@@@@@               @@@@@@@                    \n"
"                @@@@@@@@      @@@@@@@@@      @@@@@@@@                \n"
"             @@@@@@  @@@@@@@@@@@@@@@@@@@@@@@@@@@  @@@@@@             \n"
"           @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@           \n"
"          @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@          \n"
"         @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@         \n"
"        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@        \n"
"       @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@       \n"
"       @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@       \n"
"      @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@      \n"
"     @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@     \n"
"     @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@     \n"
"    @@@@@@@@@@@@@@@@@  @@@@@@@@@@@@@@@@@@@@@@@  @@@@@@@@@@@@@@@@@    \n"
"    @@@@@@@@@@@@@@@@@      @@@@@@@@@@@@@@@      @@@@@@@@@@@@@@@@@    \n"
"    @@@@@@@@@@@@@@@@@         @@@@@@@@@         @@@@@@@@@@@@@@@@@@   \n"
"   @@@@@@@@@@@@@@@@@@        @@@@@@@@@@@        @@@@@@@@@@@@@@@@@@   \n"
"   @@@@@@@@@@@@@@@@@@@@    @@@@@@@@@@@@@@@    @@@@@@@@@@@@@@@@@@@@   \n"
"   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@   \n"
"   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@   \n"
"  @@@@@@@@@@@@@@@@@@@@@@@@@   @@    @@    @@@@@@@@@@@@@@@@@@@@@@@@@  \n"
"   @@@@@@@@@@@@@@@@@@@@@@@ @@@   @@@   @@@@@@@@@@@@@@@@@@@@@@@@@@@   \n"
"    @@@@@@@@@@@@@@  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@  @@@@@@@@@@@@@@    \n"
"       @@@@@@@@@@@@@@@     @@@@@@@@@@@@@@@     @@@@@@@@@@@@@@@       \n"
"          @@@@@@@@@@@@@@                     @@@@@@@@@@@@@@          \n"
"               @@@@@@@@                       @@@@@@@@@              \n";

const char *ALRI_LOGO_COMPACT =
"          @@@@@       @@@@@          \n"
"      @@@@@   @@@@@@@   @@@@@        \n"
"    @@@@@@@@@@@@@@@@@@@@@@@@@@@      \n"
"   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@     \n"
"  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@    \n"
"  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@    \n"
"  @@@@@@@@  @@@@@@@@@  @@@@@@@@      \n"
"  @@@@@@@@    @@@@@    @@@@@@@@      \n"
"  @@@@@@@@@  @@@@@@@  @@@@@@@@@      \n"
"  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@      \n"
"   @@@@@@@@@@ @  @ @ @@@@@@@@@@      \n"
"    @@@@@@@  @@@@@@@  @@@@@@@        \n"
"       @@@@@@       @@@@@@           \n"
"         @@@         @@@             \n";

const char *ASCII_ART_FULL =
"  $$$$$$\\  $$$$$$$\\        $$$$$$$\\   $$$$$$\\        $$$$$$$\\ $$\\     $$\\ $$$$$$$\\   $$$$$$\\   $$$$$$\\   $$$$$$\\  \n"
" $$  __$$\\ $$  __$$\\       $$  __$$\\ $$  __$$\\       $$  __$$\\\\$$\\   $$  |$$  __$$\\ $$  __$$\\ $$  __$$\\ $$  __$$\\ \n"
" $$ /  $$ |$$ |  $$ |      $$ |  $$ |$$ /  \\__|      $$ |  $$ |\\$$\\ $$  / $$ |  $$ |$$ /  $$ |$$ /  \\__|$$ /  \\__|\n"
" $$$$$$$$ |$$$$$$$  |      $$ |  $$ |$$ |            $$$$$$$\\ | \\$$$$  /  $$$$$$$  |$$$$$$$$ |\\$$$$$$\\  \\$$$$$$\\  \n"
" $$  __$$ |$$  __$$<       $$ |  $$ |$$ |            $$  __$$\\   \\$$  /   $$  ____/ $$  __$$ | \\____$$\\  \\____$$\\ \n"
" $$ |  $$ |$$ |  $$ |      $$ |  $$ |$$ |  $$\\       $$ |  $$ |   $$ |    $$ |      $$ |  $$ |$$\\   $$ |$$\\   $$ |\n"
" $$ |  $$ |$$ |  $$ |      $$$$$$$  |\\$$$$$$  |      $$$$$$$  |   $$ |    $$ |      $$ |  $$ |\\$$$$$$  |\\$$$$$$  |\n"
" \\__|  \\__|\\__|  \\__|      \\_______/  \\______/       \\_______/    \\__|    \\__|      \\__|  \\__| \\______/  \\______/ \n";

const char *ASCII_ART_COMPACT =
"    _    ____  ____  _                         _ \n"
"   / \\  |  _ \\|  _ \\(_)___  ___ ___  _ __ __| |\n"
"  / _ \\ | |_) | | | | / __|/ __/ _ \\| '__/ _' |\n"
" / ___ \\|  _ <| |_| | \\__ \\ (_| (_) | | | (_| |\n"
"/_/   \\_\\_| \\_\\____/|_|___/\\___\\___/|_|  \\__,_|\n";

void printCentered(const std::string &text, int termWidth, const char *color = UI::RESET) {
  int pad = (termWidth - static_cast<int>(text.length())) / 2;
  if (pad < 0) pad = 0;
  std::cout << std::string(pad, ' ') << color << text << UI::RESET << "\n";
}

void printBannerAnimated() {
  int termWidth = UI::getTerminalWidth();

  // Oculta cursor durante a animação inicial para efeito profissional
  std::cout << "\033[?25l";
  std::cout << "\033[2J\033[H" << std::flush;

  // 1. ANIMAÇÃO DE CONSTRUÇÃO DA LOGO: Efeito "Synthesizer / Materialização"
  // As linhas surgem progressivamente de cima para baixo com brilho inicial que se fixa no tom ALRI
  if (termWidth >= 80) {
    std::istringstream logoStream(ALRI_LOGO_FULL);
    std::string line;
    while (std::getline(logoStream, line)) {
      int pad = (termWidth - static_cast<int>(line.length())) / 2;
      if (pad < 0) pad = 0;

      // Frame flash/construção
      std::cout << "\r" << std::string(pad, ' ') << UI::RED_BRIGHT << line << UI::RESET << std::flush;
      std::this_thread::sleep_for(std::chrono::milliseconds(22));

      // Fixa na cor vermelha sólida
      std::cout << "\r" << std::string(pad, ' ') << UI::RED << line << UI::RESET << "\n" << std::flush;
    }
    std::cout << "\n";

    // 2. DIGITAÇÃO / MATERIALIZAÇÃO DO TÍTULO "AR DC BYPASS"
    std::istringstream artStream(ASCII_ART_FULL);
    while (std::getline(artStream, line)) {
      int pad = (termWidth - static_cast<int>(line.length())) / 2;
      if (pad < 0) pad = 0;

      // Efeito de escaneamento horizontal rápido por caractere
      std::cout << std::string(pad, ' ') << UI::RED_BRIGHT;
      for (char ch : line) {
        std::cout << ch << std::flush;
        if (ch != ' ') {
          std::this_thread::sleep_for(std::chrono::microseconds(350));
        }
      }
      std::cout << UI::RESET << "\n";
      std::this_thread::sleep_for(std::chrono::milliseconds(18));
    }
  } else {
    // Modo Compacto para telas estreitas
    std::istringstream logoStream(ALRI_LOGO_COMPACT);
    std::string line;
    while (std::getline(logoStream, line)) {
      int pad = (termWidth - static_cast<int>(line.length())) / 2;
      if (pad < 0) pad = 0;
      std::cout << "\r" << std::string(pad, ' ') << UI::RED_BRIGHT << line << UI::RESET << std::flush;
      std::this_thread::sleep_for(std::chrono::milliseconds(25));
      std::cout << "\r" << std::string(pad, ' ') << UI::RED << line << UI::RESET << "\n" << std::flush;
    }
    std::cout << "\n";

    std::istringstream artStream(ASCII_ART_COMPACT);
    while (std::getline(artStream, line)) {
      int pad = (termWidth - static_cast<int>(line.length())) / 2;
      if (pad < 0) pad = 0;
      std::cout << std::string(pad, ' ') << UI::RED_BRIGHT << line << UI::RESET << "\n" << std::flush;
      std::this_thread::sleep_for(std::chrono::milliseconds(25));
    }
  }

  // 3. DIVISÓRIA E CABEÇALHO COM ANIMAÇÃO DE REVELAÇÃO
  int barLen = termWidth > 80 ? 80 : termWidth - 4;
  if (barLen < 20) barLen = 20;
  std::string bar = std::string(barLen, '=');
  
  std::cout << "\n";
  printCentered(bar, termWidth, UI::DIM);
  printCentered(std::string("ALRI GROUP - ARDiscordBypass v") + APP_VERSION + " (Discord Live)", termWidth, UI::BOLD);
  printCentered(std::string("Repositório Oficial: https://github.com/") + GITHUB_REPO, termWidth, UI::MUTED);
  printCentered(bar, termWidth, UI::DIM);
  std::cout << "\n" << std::flush;

  // Restaura o cursor
  std::cout << "\033[?25h" << std::flush;
}

std::string fetchLatestVersion() {
#ifdef _WIN32
  HINTERNET hSession = WinHttpOpen(L"ARDCB/3.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
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
  std::string latest = UI::spinner("Verificando atualizações no GitHub", []() {
    return fetchLatestVersion();
  });

  if (latest.empty()) {
    UI::warn("Não foi possível verificar atualizações (sem conexão com GitHub).");
    return false;
  }

  if (latest > APP_VERSION) {
    std::cout << UI::YELLOW << UI::BOLD << "[!] NOVA VERSÃO DISPONÍVEL: v" << latest
              << " (versão atual: v" << APP_VERSION << ")" << UI::RESET << std::endl;
    std::cout << UI::CYAN << "    Baixe em: https://github.com/" << GITHUB_REPO << "/releases/latest" << UI::RESET << std::endl;
    return true;
  }

  return false;
}

int main(int argc, char *argv[]) {
  UI::initConsole();
  printBannerAnimated();

#ifndef _WIN32
  if (geteuid() != 0) {
    std::string sudoCheck = runCommand("sudo -n true 2>&1");
    if (!sudoCheck.empty()) {
      UI::info("A configuração de isolamento de rede requer privilégios de superusuário (sudo).");
      int ret = std::system("sudo true");
      if (ret != 0) {
        UI::error("Privilégios sudo são necessários para isolar o tráfego do Discord.");
        return 1;
      }
    }
  }

  std::string prereqErr;
  bool prereqsOk = UI::spinner("Verificando pré-requisitos do sistema", [&prereqErr]() {
    return checkPrerequisites(prereqErr);
  });

  if (!prereqsOk) {
    UI::error("Falha nos pré-requisitos:");
    std::cerr << prereqErr << std::endl;
    return 1;
  }
#endif

#ifdef _WIN32
  WSADATA wsaData;
  WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

  checkForUpdate();
  std::cout << std::endl;

#ifdef _WIN32
  std::wstring discordExe;
  bool foundExe = UI::spinner("Localizando executável do Discord", [&]() {
    discordExe = locateDiscordExe();
    if (discordExe.empty() && argc > 1) {
      int wlen = MultiByteToWideChar(CP_UTF8, 0, argv[1], -1, nullptr, 0);
      if (wlen > 0) {
        std::wstring arg(wlen, 0);
        MultiByteToWideChar(CP_UTF8, 0, argv[1], -1, &arg[0], wlen);
        arg.resize(wlen - 1);
        discordExe = arg;
      }
    }

    if (discordExe.empty()) {
      wchar_t programFiles[MAX_PATH];
      if (GetEnvironmentVariableW(L"ProgramFiles", programFiles, MAX_PATH) != 0) {
        std::wstring path = std::wstring(programFiles) + L"\\Discord\\app-*/Discord.exe";
        WIN32_FIND_DATAW findData;
        HANDLE hFind = FindFirstFileW(path.c_str(), &findData);
        if (hFind != INVALID_HANDLE_VALUE) {
          discordExe = std::wstring(programFiles) + L"\\Discord\\" + findData.cFileName + L"\\Discord.exe";
          FindClose(hFind);
        }
      }
    }
    return !discordExe.empty();
  });

  if (!foundExe || discordExe.empty()) {
    UI::error("Não foi possível encontrar a instalação do Discord no seu computador.");
    UI::info("Dica: Você pode executar arrastando o Discord.exe para cima deste programa.");
    WSACleanup();
    return 1;
  }
#else
  std::string manualArg = (argc > 1) ? argv[1] : "";
  LaunchTarget launchTarget;
  bool foundTarget = UI::spinner("Identificando tipo de empacotamento do Discord", [&]() {
    launchTarget = locateDiscordLaunchTarget(manualArg);
    return !launchTarget.command.empty();
  });

  if (!foundTarget || launchTarget.command.empty()) {
    UI::error("Instalação do Discord não encontrada (Nativo, Flatpak, Snap ou AppImage).");
    std::cout << "  Uso manual: " << argv[0] << " /caminho/para/Discord" << std::endl;
    return 1;
  }
  UI::info("Alvo selecionado: " + launchTarget.displayName);
#endif

  // Verificação de Discord já em execução
  if (isDiscordRunning()) {
#ifndef _WIN32
    if (isDiscordRunningInNetns()) {
      UI::success("O Discord já está aberto e protegido dentro do túnel isolado!");
      std::cout << std::endl;
      UI::info("Tudo pronto! Você já pode transmitir sua tela e câmera normalmente.");
      std::cout << std::endl;
      std::cout << UI::DIM << "Pressione ENTER para fechar este assistente..." << UI::RESET;
      std::cout.flush();
      std::string dummy;
      std::getline(std::cin, dummy);
      return 0;
    }
#endif
    UI::spinner("Encerrando instâncias abertas na rede comum", []() {
      killDiscord();
    });
  }

#ifndef _WIN32
  bool tunnelOk = UI::spinner("Criando túnel WireGuard dedicado (Namespace seguro)", []() {
    return ensureWireGuardNetns();
  });

  if (!tunnelOk) {
    UI::error("Falha ao inicializar o túnel WireGuard isolado.");
    return 1;
  }

  bool launched = UI::spinner("Iniciando Discord dentro do túnel isolado", [&]() {
    return launchDiscord(launchTarget);
  });

  if (!launched) {
    UI::error("Falha ao iniciar o Discord no túnel.");
    return 1;
  }
#else
  bool tunnelOk = UI::spinner("Configurando túnel WireGuard por processo (WireSock WFP)", []() {
    return ensureWireGuardWindows();
  });

  if (!tunnelOk) {
    UI::error("Falha ao ativar o isolamento de rede do WireSock.");
    WSACleanup();
    return 1;
  }

  bool launched = UI::spinner("Iniciando Discord em segundo plano", [&]() {
    return launchDiscord(discordExe);
  });

  if (!launched) {
    UI::error("Falha ao iniciar o processo do Discord.");
    WSACleanup();
    return 1;
  }
#endif

  std::cout << std::endl;
  std::cout << UI::GREEN << UI::BOLD << "=========================================================================" << UI::RESET << std::endl;
  std::cout << UI::GREEN << UI::BOLD << "  ✔ DISCORD INICIADO COM SUCESSO! BYPASS APLICADO." << UI::RESET << std::endl;
  std::cout << UI::WHITE << "  - Sua transmissão de tela e câmera estão liberadas." << UI::RESET << std::endl;
  std::cout << UI::WHITE << "  - Todo o restante da sua máquina continua 100% na sua internet normal." << UI::RESET << std::endl;
  std::cout << UI::GREEN << UI::BOLD << "=========================================================================" << UI::RESET << std::endl;

#ifdef _WIN32
  WSACleanup();
#endif

  std::cout << std::endl;
  std::cout << UI::DIM << "Pressione ENTER para fechar este assistente..." << UI::RESET;
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
