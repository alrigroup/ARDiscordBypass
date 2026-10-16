# 📚 Documentação Técnica — ARDiscordBypass v3.5.0

> **Documentação de Engenharia de Sistemas, Arquitetura e Protocolos de Rede**  
> **ALRI GROUP (ALRI Technology Ecosystem)** — Versão 3.5.0 (2026)

---

## 📑 Índice Geral

1. [Visão Geral & Arquitetura do Sistema](#1-visão-geral--arquitetura-do-sistema)
2. [Estrutura do Repositório](#2-estrutura-do-repositório)
3. [Módulo PC (Desktop)](#3-módulo-pc-desktop)
   - [3.1 Linux — Network Namespace (`netns`) & WireGuard](#31-linux--network-namespace-netns--wireguard)
   - [3.2 Windows — Windows Filtering Platform (WFP) & Execução Segura (`CreateProcessW`)](#32-windows--windows-filtering-platform-wfp--execução-segura-createprocessw)
   - [3.3 Camada de Interface & UX CLI (C++20)](#33-camada-de-interface--ux-cli-c20)
   - [3.4 Parâmetros de Linha de Comando (CLI Parser)](#34-parâmetros-de-linha-de-comando-cli-parser)
   - [3.5 Edição Gráfica (Dear ImGui + OpenGL)](#35-edição-gráfica-dear-imgui--opengl)
   - [3.6 Arquitetura de Binário Único Autocontido (`Embedded Binary Auto-Extraction`)](#36-arquitetura-de-binário-único-autocontido-embedded-binary-auto-extraction)
   - [3.7 Handshake SRP, Normalização de Credenciais & Mapeamento de Erros](#37-handshake-srp-normalização-de-credenciais--mapeamento-de-erros)
4. [Módulo Mobile](#4-módulo-mobile)
   - [4.1 Android — VpnService com GoBackend WireGuard](#41-android--vpnservice-com-gobackend-wireguard)
   - [4.2 Split-Tunneling por Identificador de Pacote](#42-split-tunneling-por-identificador-de-pacote)
   - [4.3 Quick Settings Tile & Atualizador Assíncrono](#43-quick-settings-tile--atualizador-assíncrono)
   - [4.4 Autenticação Proton In-App via Biblioteca Nativa (`libprotonconfgen.so`)](#44-autenticação-proton-in-app-via-biblioteca-nativa-libprotonconfgenso--100-open-source)
   - [4.5 Futuro: iOS Engine (NetworkExtension)](#45-futuro-ios-engine-networkextension)
5. [Infraestrutura do Túnel & Criptografia](#5-infraestrutura-do-túnel--criptografia)
6. [Instruções de Compilação & Build](#6-instruções-de-compilação--build)
   - [6.1 Build Linux nativo com CMake & Objcopy](#61-build-linux-nativo-com-cmake--objcopy)
   - [6.2 Cross-compile MinGW para Windows com Objeto Embutido](#62-cross-compile-mingw-para-windows-com-objeto-embutido)
   - [6.3 Build do APK Android](#63-build-do-apk-android)
   - [6.4 Compilação do Motor Go para Android (`libprotonconfgen.so`)](#64-compilação-do-motor-go-para-android-libprotonconfgenso)
7. [Segurança, Antiban e Relatórios VirusTotal](#7-segurança-antiban-e-relatórios-virustotal)
8. [Licença & Termos de Uso](#8-licença--termos-de-uso)

---

## 1. Visão Geral & Arquitetura do Sistema

O **ARDiscordBypass v3.5.0** foi concebido para contornar instabilidades e bloqueios de infraestrutura de mídia RTC (WebRTC) e transmissões de tela/vídeo do Discord no território brasileiro, sem afetar o tráfego global da máquina e sem necessitar de VPN de sistema integral.

```
                              [ Aplicações do Sistema ]
                   WhatsApp / Navegadores / Jogos / Steam / Spotify
                                         │
                                         ▼ (Conexão Direta à Internet)
                             ┌───────────────────────┐
                             │  Interface WAN Nativa │  (0ms de latência extra)
                             └───────────────────────┘
                                         ▲
      [ Discord Desktop / Mobile ]       │
                   │                     │
                   ▼ (Roteamento Seletivo)│
      ┌─────────────────────────┐        │
      │ ARDiscordBypass Engine  │────────┘ (Split-Tunneling Estrito)
      │  (WireGuard Isolated)   │
      └────────────┬────────────┘
                   │
                   ▼ (Túnel UDP Criptografado ChaCha20-Poly1305)
      ┌─────────────────────────┐
      │   Servidor WireGuard    │
      │   (Gateway de Mídia)    │
      └─────────────────────────┘
```

---

## 2. Estrutura do Repositório

O projeto é estruturado em domínios isolados por plataforma com código 100% aberto:

```
ARDiscordBypass/
├── DOCS.md                     # Esta documentação técnica de engenharia
├── README.md                   # Apresentação do projeto e guia do usuário
├── LICENSE                     # Termos da licença ARGLFU v2
├── logo.png                    # Identidade visual oficial ALRI
├── ARDiscordBypass_linux       # Executável CLI Linux x86_64 (Single Standalone com gerador embutido)
├── ARDiscordBypass_gui_linux   # Executável GUI Desktop Linux com CLI embutido e Dear ImGui
├── ARDiscordBypass.exe         # Executável CLI Windows x64 (Single Standalone com gerador embutido)
├── ARDiscordBypass_gui.exe     # Executável GUI Desktop Windows com CLI embutido e Dear ImGui
├── ARDiscordBypass.apk         # Aplicativo Android compilado
│
├── PC/                         # Solução Desktop em C++20 Nativo
│   ├── CMakeLists.txt          # Configuração CMake do módulo PC (com regras objcopy de embedding)
│   ├── build.sh                # Script de compilação automatizada Linux
│   ├── build.bat               # Script de compilação automatizada Windows
│   ├── main.cpp                # Ponto de entrada, verificador e renderizador CLI
│   ├── ui.h                    # Paleta TrueColor ANSI, Braille spinners e responsividade
│   ├── core/service.h          # Gerenciador de túnel, resolução multi-path e processos
│   ├── bin/                    # Binários auxiliares utilizados na etapa de build/linkagem
│   │   ├── proton-confgen      # Binário ELF x86_64 para linkagem estática no Linux
│   │   ├── proton-confgen.exe  # Binário PE x64 para linkagem estática no Windows
│   │   ├── wiresock-client.exe # Cliente de injeção WFP WireGuard para Windows
│   │   └── ws_driver.sys       # Driver de kernel de filtragem WFP
│   ├── gui/                    # Interface gráfica OpenGL Dear ImGui (Dark/Light)
│   │   ├── app_window.cpp      # Janela principal, modais de setup, notas e créditos
│   │   ├── app_window.h        # Cabeçalho da aplicação GUI
│   │   ├── widgets.h           # Componentes visuais personalizados
│   │   └── style.h             # Paleta de cores ALRI Red e temas
│   └── os/
│       ├── linux/platform.h    # Namespaces (`netns`), auto-extração (~/.local/share) e WireGuard
│       └── windows/platform.h  # CreateProcessW nativo, auto-extração (%LOCALAPPDATA%) e WFP WireSock
│
├── tools/                      # Ferramentas Auxiliares de Código Aberto
│   └── proton-confgen/         # Código-Fonte do Motor Proton / SRP (Go 1.22+)
│       ├── Makefile            # Automação de builds multi-plataforma
│       ├── cmd/protonvpn-wg/   # Ponto de entrada CLI (main.go com DNS fallback)
│       ├── internal/auth/      # Handshake SRP oficial da Proton e renovação de sessão
│       ├── internal/api/       # Transporte HTTP/TLS com emulação uTLS
│       ├── internal/vpn/       # Descoberta de servidores internacionais gratuitos
│       └── internal/wireguard/ # Geração de chaves Curve25519 e perfil ardc_vpn.conf
│
└── Mobile/                     # Soluções Mobile Nativas
    └── Android/                # App Android (Kotlin, Jetpack, WireGuard GoBackend)
        ├── app/
        │   ├── build.gradle.kts
        │   └── src/main/
        │       ├── AndroidManifest.xml
        │       ├── jniLibs/arm64-v8a/
        │       │   └── libprotonconfgen.so         # Binário nativo compilado de tools/proton-confgen
        │       ├── java/com/alrigroup/ardcb/
        │       │   ├── MainActivity.kt             # Terminal interativo OLED, abas e diálogos
        │       │   ├── service/DiscordVpnService.kt# VpnService em primeiro plano
        │       │   ├── tunnel/WireGuardConfigManager.kt # Split-tunneling por UID
        │       │   ├── tunnel/ProtonAuthManager.kt # Orquestrador nativo do gerador Go
        │       │   ├── tile/BypassQuickTile.kt     # Botão nas Configurações Rápidas
        │       │   └── updater/UpdateChecker.kt    # Validador de releases assíncrono
        │       └── res/                            # Layouts terminal, cores e temas
        ├── gradle/
        └── build.gradle.kts
```

---

## 3. Módulo PC (Desktop)

### 3.1 Linux — Network Namespace (`netns`) & WireGuard

No Linux, a técnica padrão de VPN altera a tabela de roteamento da máquina (`ip route replace default`), afetando todos os programas abertos. O ARDiscordBypass utiliza a funcionalidade de **Network Namespaces** do kernel Linux (`ip netns`):

1. **Criação do Namespace Isolado**:
   Um namespace chamado `ar-discord-vpn` é criado via `ip netns add ar-discord-vpn`.
2. **Interface WireGuard Virtual**:
   Uma interface `wg-ardc` é instanciada e movida para o namespace:
   ```bash
   ip link add wg-ardc type wireguard
   ip link set wg-ardc netns ar-discord-vpn
   ```
3. **Endereçamento e Roteamento Dedicado**:
   Dentro do namespace, atribui-se o IP privado `10.7.0.2/24` e define-se a rota padrão para a interface WireGuard.
4. **Execução sem Vazamento**:
   O binário do Discord (nativo, Flatpak ou Snap) é lançado dentro do namespace via:
   ```bash
   ip netns exec ar-discord-vpn sudo -u $USUARIO_REAL env ... discord
   ```
   Dessa forma, **apenas o Discord** e seus subprocessos Electron navegam pelo túnel; todos os demais softwares do Linux continuam conectados via Ethernet/Wi-Fi normal.

### 3.2 Windows — Windows Filtering Platform (WFP) & Execução Segura (`CreateProcessW`)

No Windows, namespaces de rede não existem no mesmo formato do Linux. A solução de nível profissional é implementada através do **Windows Filtering Platform (WFP)** com motor WireSock, gerenciado por chamadas diretas da Win32 API:

1. **Filtro em Modo Kernel (`ws_driver.sys`)**:
   O WireSock intercepta pacotes na camada de transporte (WFP) antes que atinjam a pilha NDIS do Windows.
2. **Diretiva de Isolamento de Aplicação**:
   O arquivo de configuração gerado dinamicamente define:
   ```ini
   [Interface]
   PrivateKey = ...
   Address = 10.7.0.2/24
   DNS = 1.1.1.1
   #@ws:AllowedApps = Discord, DiscordCanary, DiscordPTB, Update
   ```
3. **Execução Segura de Subprocessos via Win32 API (`CreateProcessW` vs `_popen`)**:
   - **Vulnerabilidade do `_popen` Clássico**: A função tradicional `_popen` repassa a linha de comando para o interpretador de lote `cmd.exe /c "..."`. O `cmd.exe` possui regras complexas de manipulação de aspas duplas, removendo aspas externas quando detecta múltiplos argumentos citados (quebrando caminhos com espaços como `C:\Program Files\...`). Adicionalmente, caracteres especiais em senhas fortes (tais como `&`, `%`, `^`, `<`, `>`, `|`) são interpretados como operadores de redirecionamento ou variáveis de ambiente pelo shell, corrompendo a autenticação SRP.
   - **Solução Nativa Robusta (`CreateProcessW` + Anonymous Pipes)**:
     O ARDiscordBypass substituiu totalmente chamadas de shell por invocação direta de processo através de `CreateProcessW`:
     - Argumentos formatados com escape seguro de aspas duplas.
     - Criação de canais anônimos unidirecionais (`CreatePipe`) com atributo de segurança `bInheritHandle = TRUE` para capturar `stdout` e `stderr`.
     - Flag `CREATE_NO_WINDOW`, eliminando qualquer piscada indesejada de janela preta do console para o usuário final.
     - Leitura não-bloqueante em streaming via `ReadFile` (buffers de 4 KB), alimentando os callbacks de progresso em tempo real da interface gráfica e do terminal.
4. **Desanexação de Handles e Limpeza de Console**:
   O processo do Discord e do WireSock são instanciados com as flags `DETACHED_PROCESS` e `CREATE_BREAKAWAY_FROM_JOB`, redirecionando handles residuais para `NUL`. Isso impede qualquer vazamento de logs do Electron no console do bypass.

### 3.3 Camada de Interface & UX CLI (C++20)

- **Detecção de Dimensão do Terminal**: Realizada via `ioctl(TIOCGWINSZ)` no Linux e `GetConsoleScreenBufferInfo` no Windows.
- **Renderização Adaptativa**: Alterna entre a arte ASCII completa (para telas ≥ 80 colunas) e a arte compactada (para terminais estreitos), prevenindo quebras visuais.
- **Animações de Escrita e Loading**: A logo é construída em camadas com efeitos de pulso de brilho e os spinners operam assincronamente através de `std::async` e `std::atomic<bool>`.

### 3.4 Parâmetros de Linha de Comando (CLI Parser)

O ponto de entrada `PC/main.cpp` processa argumentos de linha de comando:

- `-stop`, `--stop`: Executa `stopBypass()`, enviando sinais de terminação a processos órfãos (`ip netns pids`), eliminando instâncias do Discord, destruindo interfaces `wg-ardc`, removendo o namespace `ar-discord-vpn` (ou o serviço WireSock no Windows) e limpando travas singleton (`SingletonLock`, `SingletonCookie`, `SingletonSocket`).
- `-cfg <path>`, `--config <path>`: Permite apontar para qualquer arquivo de configuração WireGuard arbitrário, ignorando a renovação automática da ProtonVPN e injetando as diretivas necessárias de interface e MTU.
- `-l`, `--login`, `--proton`: Força reautenticação interativa via SRP com o utilitário `proton-confgen`.
- `-h`, `--help`: Exibe a especificação de sintaxe e exemplos de uso.

### 3.5 Edição Gráfica (Dear ImGui + OpenGL)

A variante gráfica opcional (`PC/gui/`) fornece uma experiência visual moderna e reativa, compartilhando o mesmo Design System do aplicativo Android:

- **Unificação Visual (Light / Dark Mode idêntico ao Android)**:
  - **Dark Mode**: Fundo 100% OLED Pure Black (`#000000`), superfície `#0A0A0A`, cards `#121212`, bordas `#242424` e realce em Vermelho ALRI (`#E50914`).
  - **Light Mode**: Superfície clara `#F8FAFC`, cards `#F1F5F9`, bordas `#E2E8F0` e tipografia de alto contraste `#0F172A`.
  - Botão de alternância instantânea no cabeçalho com ícones procedurais de sol/lua.
- **Logo Oficial Incorporada na Compilação**:
  - A imagem da logo ALRI (`logo.png`) é convertida em array binário C (`logo_data.h`) e compilada diretamente no executável. O carregamento ocorre via `stb_image` para textura RGBA OpenGL em memória, tornando o executável 100% autocontido (independente de arquivos externos na pasta).
- **Ícones Vetoriais Procedurais (Zero Glifos Corrompidos `?`)**:
  - Eliminação de dependência de emojis de fontes UTF-8 externas que causavam pontos de interrogação no Linux. Os ícones (Play, Stop, Sol, Lua, Check, Status) são desenhados diretamente pelo motor vetorial `ImDrawList`.
- **Animações Fluidas de Loading & Status**:
  - **Spinner Circular Rotativo**: Arco suave girando a 60 FPS durante operações assíncronas.
  - **Ponto Pulsante de Status**: Anel dinâmico em onda senoidal no indicador de túnel ativo.
  - **Barra de Progresso Indeterminada**: Efeito shimmer com transição contínua.
  - **Terminal Retrô com Cursor Piscante**: Exibição de logs em caixa monospace com prompt `alri@ardcb:~$` e cursor piscante a cada 500ms.
- **Renderização e Backends**:
  - No Linux: `GLFW 3` + `OpenGL 3` com suspensão de eventos ociosa (`glfwWaitEventsTimeout`), mantendo 0% de uso de CPU/GPU em repouso.
  - No Windows: Win32 puro (`imgui_impl_win32`) + `OpenGL 3`, gerando um binário 100% estático sem exigir nenhuma DLL adicional.
- **Assincronia & Responsividade**: A inicialização do túnel, negociação e testes de rede ocorrem em threads dedicadas (`std::async`), mantendo a UI fluida e sem travamentos ("Não Respondendo").
- **Seletor Nativo de Arquivos**: Integração com as caixas de diálogo do sistema operacional (`GetOpenFileNameW` no Windows e `zenity` / `kdialog` no Linux).

### 3.6 Arquitetura de Binário Único Autocontido (`Embedded Binary Auto-Extraction`)

Em versões legadas e ferramentas convencionais de split-tunneling, a distribuição exigia múltiplos arquivos soltos em pastas relativas (como `bin/wiresock-client.exe`, `bin/proton-confgen.exe`, scripts `.bat` e arquivos de apoio). Caso o usuário final baixasse apenas o executável avulso em sua pasta `Downloads` ou o movesse para a Área de Trabalho, o programa falhava instantaneamente por caminhos relativos quebrados.

O **ARDiscordBypass v3.5.0** implementa o padrão **Single Standalone Executable**:

1. **Linkagem de Objeto Binário Bruto (`objcopy`)**:
   Durante a fase de compilação, o utilitário completo de geração de chaves e autenticação Proton é convertido diretamente em um arquivo objeto compilável:
   - **No Windows (PE-COFF x64)**:
     ```cmd
     objcopy -I binary -O pe-x86-64 -B i386:x86-64 proton-confgen.exe proton_confgen_embed.o
     ```
     Exporta os ponteiros globais:
     ```cpp
     extern "C" const char _binary_proton_confgen_exe_start[];
     extern "C" const char _binary_proton_confgen_exe_end[];
     ```
   - **No Linux (ELF x86_64)**:
     ```bash
     objcopy -I binary -O elf64-x86-64 -B i386:x86-64 \
       --rename-section .data=.rodata,alloc,load,readonly,data,contents \
       proton-confgen proton_confgen_embed.o
     ```
     Exporta os ponteiros globais:
     ```cpp
     extern "C" const char _binary_proton_confgen_start[];
     extern "C" const char _binary_proton_confgen_end[];
     ```
2. **Auto-Extração Atômica em Tempo de Execução (`ensureProtonConfgenAvailable`)**:
   Na primeira chamada da camada de serviço (`loginProton` ou verificação de chaves):
   - O aplicativo localiza a pasta padrão de dados de aplicativos do sistema:
     - **Windows**: `%LOCALAPPDATA%\ARDiscordBypass\proton-confgen.exe`
     - **Linux**: `~/.local/share/ARDiscordBypass/proton-confgen`
   - Se o utilitário já existir e seu tamanho for íntegro (> 1 MB), a extração é ignorada, prosseguindo com execução imediata.
   - Caso não exista, o diretório é criado automaticamente (`CreateDirectoryW` / `mkdir -p`), o payload bruto embutido em memória de leitura (`.rodata`) é gravado em disco de forma atômica e as permissões de execução adequadas (`chmod 0755` no Linux) são atribuídas.
3. **Independência Operacional**:
   O usuário pode colocar o `ARDiscordBypass.exe` ou `ARDiscordBypass_gui.exe` em qualquer diretório arbitrário, pen-drive ou pasta compartilhada sem necessidade de instalar pastas acessórias ou configuradores externos.

### 3.7 Handshake SRP, Normalização de Credenciais & Mapeamento de Erros

A comunicação com os servidores de identidade da ProtonVPN segue o protocolo criptográfico oficial **SRP (Secure Remote Password - RFC 5054)**, impedindo o tráfego da senha em texto claro. O módulo de autenticação foi aperfeiçoado para garantir resiliência contra exceções comuns de uso:

1. **Normalização Case-Insensitive de Nomes de Usuário**:
   Usuários frequentemente preenchem identificadores com diferentes grafias ou caixas (`@Proton.me`, `@PM.ME`, `@protonmail.com`). A camada de validação sanitiza e normaliza o identificador:
   ```go
   // Remove sufixos Proton ignorando maiúsculas e minúsculas
   cleanUsername := strings.TrimSpace(username)
   lower := strings.ToLower(cleanUsername)
   for _, suffix := range []string{"@proton.me", "@protonmail.com", "@pm.me"} {
       if strings.HasSuffix(lower, suffix) {
           cleanUsername = cleanUsername[:len(cleanUsername)-len(suffix)]
           break
       }
   }
   ```
   Emails de outros provedores (ex: `@gmail.com`, `@outlook.com`) vinculados à conta Proton gratuita são preservados integralmente.
2. **Tratamento Humanizado de Códigos de Erro da API**:
   Respostas JSON de erro da API Proton são interceptadas e mapeadas em mensagens intuitivas:
   - **Código `8002`**: *"Senha incorreta. Verifique suas credenciais Proton."*
   - **Código `9001`**: *"Usuário não encontrado. Verifique seu login ou crie uma conta Proton gratuita."*
   - **Código `2001`**: *"Verificação humana necessária (reCAPTCHA) ou conta temporariamente limitada. Acesse account.protonvpn.com pelo navegador."*
   - **Timeout / DNS**: *"Falha de comunicação ou timeout da API Proton. Verifique sua conexão com a internet."*
3. **Resolução de Perfil Multi-Path & Persistência Anual**:
   - O arquivo de configuração gerado `ardc_vpn.conf` é sincronizado automaticamente entre a pasta de trabalho atual e o diretório de execução.
   - O token da sessão SRP é gravado em `~/.protonvpn-session.json` (`%USERPROFILE%\.protonvpn-session.json`), com permissões restritas e validade de até 1 ano (ex: 2027), eliminando a necessidade de re-digitar credenciais em cada inicialização.

---

## 4. Módulo Mobile

### 4.1 Android — VpnService com GoBackend WireGuard

O aplicativo Android utiliza a biblioteca oficial do WireGuard integrada em Go (`com.wireguard.android:tunnel:1.0.20230427`), rodando sob a API `VpnService` do Android:

```kotlin
val intent = VpnService.prepare(context)
if (intent == null) {
    // Permissão já concedida pelo usuário
    startVpnServiceInternal()
}
```

### 4.2 Split-Tunneling por Identificador de Pacote

Para não consumir os dados móveis do usuário nem afetar bancos, WhatsApp ou streaming:
```kotlin
val builder = VpnService.Builder()
val targetPackages = listOf(
    "com.discord",
    "com.discord.canary",
    "com.discord.ptb",
    "com.aliucord",
    "com.vencord"
)
for (pkg in targetPackages) {
    try {
        builder.addAllowedApplication(pkg)
    } catch (ignored: PackageManager.NameNotFoundException) { }
}
```
Apenas o tráfego originado por esses UIDs é capturado pela interface `tun0`.

### 4.3 Quick Settings Tile & Atualizador Assíncrono

- **Quick Settings Tile**: Usuários com Android 7.0+ podem adicionar o botão rápido do ARDiscordBypass na central de notificações, ligando ou desligando o túnel com um toque sem abrir a interface.
- **UpdateChecker**: Validação assíncrona contra a API REST do GitHub Releases (`https://api.github.com/repos/alrigroup/ARDiscordBypass/releases/latest`), notificando o usuário sem travar a thread de interface.

### 4.4 Autenticação Proton In-App via Biblioteca Nativa (`libprotonconfgen.so`) — 100% Open-Source

O aplicativo Android incorpora o motor nativo `libprotonconfgen.so` empacotado em `Mobile/Android/app/src/main/jniLibs/arm64-v8a/`.

#### 🔍 Transparência do Código-Fonte e Origem
- **Código 100% Aberto e Local**: Este arquivo **NÃO** é uma biblioteca binária proprietária de terceiros nem código fechado. Seu código-fonte reside integralmente na pasta **`tools/proton-confgen/`** deste mesmo repositório e é licenciado como software livre.
- **Ponto de Entrada**: Implementado em Go 1.22+ em `tools/proton-confgen/cmd/protonvpn-wg/main.go`.
- **Módulos de Criptografia & API**:
  - `internal/auth/`: Implementação do protocolo SRP (*Secure Remote Password*, RFC 5054) com geração local de segredos efêmeros e cálculo de hashes SHA-512.
  - `internal/api/`: Transporte HTTPS com biblioteca uTLS para evitar bloqueios de WAF/Cloudflare nos gateways da Proton.
  - `internal/vpn/`: Descoberta assíncrona da lista de nós da Proton com seleção de servidores gratuitos de baixa carga.
  - `internal/wireguard/`: Geração de pares de chaves assimétricas Curve25519 (ECDH) e estruturação do arquivo de perfil `ardc_vpn.conf`.

#### 🛡️ Por que o arquivo é nomeado `.so` e colocado em `jniLibs`? (Restrições de W^X no Android)
No Android 10 (API 29) em diante, a Google introduziu no kernel Linux políticas estritas de segurança denominadas **W^X** (*Write XOR Execute*) somadas a restrições de **SELinux**:
1. **Bloqueio de Executáveis em Armazenamento Privado**: Se um binário executável comum for copiado ou extraído para `/data/data/<package>/` ou `/data/user/0/<package>/files/`, qualquer tentativa de invocação via `ProcessBuilder` ou `Runtime.getRuntime().exec()` falha imediatamente com o erro do sistema:
   ```
   java.io.IOException: Cannot run program: error=13, Permission denied (EACCES)
   ```
   Mesmo que o aplicativo execute chamadas nativas de `chmod 755`, o SELinux bloqueia chamadas de sistema `execve()` e `mmap(PROT_EXEC)` em partições montadas com a flag `noexec`.
2. **A Solução Canônica Aprovada pelo Android**: A única forma legítima em sistemas Android modernos (sem necessidade de privilégios de Root) de executar código de apoio compilado a partir de código nativo (C/C++/Go/Rust) é empacotá-lo no diretório `jniLibs/<abi>/` com a nomenclatura padrão de bibliotecas compartilhadas:
   ```
   lib<nome_do_modulo>.so
   ```
3. **Mecanismo de Instalação do Sistema Operacional**:
   Com a diretiva `useLegacyPackaging = true` no `build.gradle.kts` (equivalente ao `android:extractNativeLibs="true"` no Manifest), o próprio instalador de pacotes do sistema Android (`PackageManagerService`) extrai o arquivo durante a instalação para a pasta do sistema:
   ```
   /data/app/~~.../com.alrigroup.ardcb-.../lib/arm64/libprotonconfgen.so
   ```
   Essa pasta é montada pelo kernel Android com atributos de execução legítimos (`PROT_EXEC` / `+x`), permitindo que o `ProtonAuthManager.kt` execute o binário gerador diretamente em tempo de execução via `ProcessBuilder`.

#### 🌐 Fallback de DNS Customizado para Go Puro no Android
No ecossistema Android, compilar em Go puro sem CGO (`CGO_ENABLED=0`) gerava falha de resolução DNS (`lookup api.protonvpn.ch: no such host` ou `failed to get auth info`), pois o Go puro tenta ler o arquivo `/etc/resolv.conf`, que **não existe** no sistema de arquivos do Android (onde o DNS é gerenciado pelo daemon `netd`).

Para contornar isso de forma 100% confiável, o código-fonte em `tools/proton-confgen/cmd/protonvpn-wg/main.go` injeta diretamente na inicialização do runtime um resolver customizado com fallback em servidores DNS públicos via UDP:
```go
func init() {
	net.DefaultResolver = &net.Resolver{
		PreferGo: true,
		Dial: func(ctx context.Context, network, address string) (net.Conn, error) {
			var d net.Dialer
			conn, err := d.DialContext(ctx, "udp", "1.1.1.1:53")
			if err != nil {
				return d.DialContext(ctx, "udp", "8.8.8.8:53")
			}
			return conn, nil
		},
	}
}
```

### 4.5 Futuro: iOS Engine (NetworkExtension)

A estrutura de pastas `Mobile/` foi projetada para receber o projeto Swift/Xcode utilizando `NETunnelProvider` com motor WireGuardKit para iOS.

---

## 5. Infraestrutura do Túnel & Criptografia

| Parâmetro | Valor / Especificação |
| :--- | :--- |
| **Protocolo** | WireGuard (UDP) |
| **Criptografia Simétrica** | ChaCha20 com autenticação Poly1305 (RFC 7539) |
| **Troca de Chaves** | Curve25519 (ECDH) com pares de chaves exclusivos |
| **Função de Hash** | BLAKE2s |
| **Endpoint do Gateway** | Dinâmico (ProtonVPN US/NL/JP) ou definido em `ardc_vpn.conf` |
| **MTU** | 1280 (Garante passagem sem fragmentação) com TCP MSS Clamping |
| **Keepalive** | 25 segundos |
| **Autenticação de Sessão** | Tokens locais em `~/.protonvpn-session.json` (permissão `0600`) |

O sistema não utiliza chaves globais ou credenciais embutidas no código para mitigar riscos de colisão de pacotes, saturação de tráfego e bloqueios de infraestrutura. Cada usuário gera seu próprio túnel internacional gratuito através da API ProtonVPN ou utiliza sua configuração WireGuard pessoal.

---

## 6. Instruções de Compilação & Build

### 6.1 Build Linux nativo com CMake & Objcopy

Requisitos: GCC 11+ ou Clang 13+ com suporte a C++20, CMake 3.16+, binutils (`objcopy`), e opcionalmente `glfw3` e `opengl` caso compile a interface gráfica.

O script `build.sh` e o `CMakeLists.txt` cuidam automaticamente de converter o binário `bin/proton-confgen` em objeto ELF embutido (`proton_confgen_embed.o`) e linká-lo diretamente aos executáveis finais:

**Modo Padrão (Apenas CLI ultra-leve):**
```bash
cd PC
chmod +x build.sh
./build.sh
# Gera: ../ARDiscordBypass_linux (~13 MB com motor Proton 100% embutido)
```

**Modo Completo (CLI + GUI juntas):**
```bash
cd PC
./build.sh --gui
# Gera:
#   ../ARDiscordBypass_linux      (~13 MB - Single Executable CLI)
#   ../ARDiscordBypass_gui_linux  (~14 MB - Single Executable GUI Dear ImGui)
```

Ou manualmente pelo CMake:
```bash
cd PC
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_GUI=ON
cmake --build build --parallel
```

### 6.2 Cross-compile MinGW para Windows com Objeto Embutido

No Linux (com `mingw-w64` instalado) ou no Windows nativo (via `build.bat`):

**Passo 1: Converter o utilitário Go em Objeto PE-COFF x64:**
```bash
cd PC/bin
x86_64-w64-mingw32-objcopy -I binary -O pe-x86-64 -B i386:x86-64 proton-confgen.exe ../proton_confgen_embed.o
cd ..
```

**Passo 2: Compilar a CLI (`ARDiscordBypass.exe`):**
```bash
x86_64-w64-mingw32-g++ -std=c++20 -O3 \
  -static -static-libgcc -static-libstdc++ \
  -I. main.cpp proton_confgen_embed.o \
  -o ../ARDiscordBypass.exe \
  -lws2_32 -lwinhttp
x86_64-w64-mingw32-strip ../ARDiscordBypass.exe
```

**Passo 3: Compilar a GUI (`ARDiscordBypass_gui.exe`):**
```bash
x86_64-w64-mingw32-g++ -std=c++20 -O3 \
  -static -static-libgcc -static-libstdc++ \
  gui/main.cpp gui/app_window.cpp \
  proton_confgen_embed.o \
  gui/vendor/imgui/*.cpp \
  gui/vendor/imgui/backends/imgui_impl_win32.cpp \
  gui/vendor/imgui/backends/imgui_impl_opengl3.cpp \
  -I . -I gui -I gui/vendor/imgui \
  -lopengl32 -lgdi32 -luser32 -lcomdlg32 -lshell32 -ldwmapi -limm32 -lws2_32 -lwinhttp \
  -mwindows -o ../ARDiscordBypass_gui.exe
x86_64-w64-mingw32-strip ../ARDiscordBypass_gui.exe
```

*(No Windows, você pode simplesmente abrir o Prompt de Comando na pasta `PC` e executar `build.bat` ou `build.bat --gui` para compilar tudo automaticamente).*

### 6.3 Build do APK Android

Requisitos: JDK 17 e Android SDK com Platform-Tools.

```bash
cd Mobile/Android
./gradlew assembleDebug
# O APK será gerado em app/build/outputs/apk/debug/app-debug.apk
```

Para instalar diretamente via ADB:
```bash
adb install -r app/build/outputs/apk/debug/app-debug.apk
```

### 6.4 Compilação do Motor Go para Android (`libprotonconfgen.so`)

Requisitos: Go 1.22+ instalado no sistema operacional.

Para auditar ou recompilar o binário nativo do gerador Proton utilizado pelo APK Android a partir do código-fonte:

```bash
# 1. Acessar a pasta com o código Go aberto
cd tools/proton-confgen

# 2. Compilar especificando o target Android ARM64 em modo estático sem CGO
CGO_ENABLED=0 GOOS=android GOARCH=arm64 go build -ldflags "-s -w" -o libprotonconfgen.so cmd/protonvpn-wg/main.go

# 3. Mover o binário gerado para a pasta jniLibs do projeto Android
cp libprotonconfgen.so ../../Mobile/Android/app/src/main/jniLibs/arm64-v8a/libprotonconfgen.so

# 4. (Opcional) Auditar o formato do binário ELF compilado
file libprotonconfgen.so
# Retorno esperado: ELF 64-bit LSB shared object, ARM aarch64, version 1 (SYSV), statically linked

readelf -h libprotonconfgen.so
# Retorno esperado: Machine: AArch64, OS/ABI: UNIX - System V
```

---

## 7. Segurança, Antiban e Relatórios VirusTotal

1. **Zero Modificação de Binários**: Não há injeção de DLLs, ganchos em memória (*API hooking*) ou modificações no `app.asar` ou no APK do Discord.
2. **Conformidade com os Termos do Discord**: O Discord permite conexões através de redes corporativas, VPNs e túneis proxy. O método utilizado replica um ambiente de rede normal.
3. **Privacidade de Dados**: O túnel não realiza inspeção de pacotes (*Deep Packet Inspection*) e não descriptografa payloads TLS/HTTPS (WebRTC DTLS/SRTP são mantidos ponta a ponta).
4. **Relatório Oficial VirusTotal & Integridade Criptográfica (v3.5.0)**:

| Plataforma | Edição | Binário | Status VirusTotal | Hash SHA-256 Oficial |
| :--- | :---: | :--- | :---: | :--- |
| **Android** | Mobile | `ARDiscordBypass.apk` | [**0/75 Detections (100% Limpo)**](https://www.virustotal.com/gui/file/3b4891d8e91de6689b1523c59e2aa8f291e8b40363c69efdb89a70668682db2c?nocache=1) | `3b4891d8e91de6689b1523c59e2aa8f291e8b40363c69efdb89a70668682db2c` |
| **Linux (x86_64)** | CLI | `ARDiscordBypass_linux` | [**0/75 Detections (100% Limpo)**](https://www.virustotal.com/gui/file/c1675e9cbadda07ba2e7535981bd80e942bfd0c46796fcd35d980dd2860761f1?nocache=1) | `c1675e9cbadda07ba2e7535981bd80e942bfd0c46796fcd35d980dd2860761f1` |
| **Linux (x86_64)** | GUI | `ARDiscordBypass_gui_linux` | [**0/75 Detections (100% Limpo)**](https://www.virustotal.com/gui/file/6e6cb8b1e5815ef4faefd02a5b75502fa85ca81e0b81d3c480e2949bf3bfea8e?nocache=1) | `6e6cb8b1e5815ef4faefd02a5b75502fa85ca81e0b81d3c480e2949bf3bfea8e` |
| **Windows (x64)** | CLI | `ARDiscordBypass.exe` | [**4/75 Detections (Falso Positivo)**](https://www.virustotal.com/gui/file/df9983198147fb850b473ecd58f84fac72059eb9e75c54e31801c25ed1c7cecd?nocache=1) | `df9983198147fb850b473ecd58f84fac72059eb9e75c54e31801c25ed1c7cecd` |
| **Windows (x64)** | GUI | `ARDiscordBypass_gui.exe` | [**3/75 Detections (Falso Positivo)**](https://www.virustotal.com/gui/file/0907e8d7e9ea1927fc6e5cfd35b59169d2cafe23f3a60832430031690e4468fa?nocache=1) | `0907e8d7e9ea1927fc6e5cfd35b59169d2cafe23f3a60832430031690e4468fa` |

5. **Análise Técnica dos Falsos Positivos no Windows**:
   - **Binário Autocontido (Payload PE embutido via `objcopy`)**: O executável embute o arquivo PE completo do utilitário `proton-confgen.exe` e o extrai dinamicamente para `%LOCALAPPDATA%`. Antivírus heurísticos de segunda linha frequentemente sinalizam executáveis que gravam e iniciam outros arquivos `.exe` em pastas do usuário.
   - **Linkagem Estática Completa (`-static`)**: Para garantir que o programa rode sem exigir a instalação de runtimes externos ou pacotes de redistribuição da Microsoft, as bibliotecas da libc/libstdc++ são embutidas diretamente no binário.
   - **Manipulação de Processos e Driver de Rede**: A invocação do WireSock (`ws_driver.sys`) e a inicialização desanexada do Discord (`CreateProcessW` com `DETACHED_PROCESS`) geram flags em scanners comportamentais estáticos.
   - **Conclusão**: Todos os motores industriais de referência (**Microsoft Defender, Kaspersky, Bitdefender, ESET, Sophos**) classificam os binários como **100% limpos e seguros**.

---

## 8. Licença & Termos de Uso

Este projeto é protegido sob os termos da **ARGLFU (ALRI GROUP LICENSE FREE USE) Version 2 – 2026**.

- Permitido o uso pessoal, gratuito e redistribuição na forma original.
- Proibida a engenharia reversa para fins comerciais ou monetização deste software.

**ALRI GROUP** — *Inovação, Segurança e Performance.*
