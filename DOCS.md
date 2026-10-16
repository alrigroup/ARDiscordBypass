# 📚 Documentação Técnica — ARDiscordBypass v3.0

> **Documentação de Engenharia de Sistemas, Arquitetura e Protocolos de Rede**  
> **ALRI GROUP (ALRI Technology Ecosystem)** — Versão 3.0.0 (2026)

---

## 📑 Índice Geral

1. [Visão Geral & Arquitetura do Sistema](#1-visão-geral--arquitetura-do-sistema)
2. [Estrutura do Repositório](#2-estrutura-do-repositório)
3. [Módulo PC (Desktop)](#3-módulo-pc-desktop)
   - [3.1 Linux — Network Namespace (`netns`) & WireGuard](#31-linux--network-namespace-netns--wireguard)
   - [3.2 Windows — Windows Filtering Platform (WFP) via WireSock](#32-windows--windows-filtering-platform-wfp-via-wiresock)
   - [3.3 Camada de Interface & UX CLI (C++20)](#33-camada-de-interface--ux-cli-c20)
4. [Módulo Mobile](#4-módulo-mobile)
   - [4.1 Android — VpnService com GoBackend WireGuard](#41-android--vpnservice-com-gobackend-wireguard)
   - [4.2 Split-Tunneling por Identificador de Pacote](#42-split-tunneling-por-identificador-de-pacote)
   - [4.3 Quick Settings Tile & Atualizador Assíncrono](#43-quick-settings-tile--atualizador-assíncrono)
   - [4.4 Futuro: iOS Engine (NetworkExtension)](#44-futuro-ios-engine-networkextension)
5. [Infraestrutura do Túnel & Criptografia](#5-infraestrutura-do-túnel--criptografia)
6. [Instruções de Compilação & Build](#6-instruções-de-compilação--build)
   - [6.1 Build Linux nativo](#61-build-linux-nativo)
   - [6.2 Cross-compile MinGW para Windows](#62-cross-compile-mingw-para-windows)
   - [6.3 Build do APK Android](#63-build-do-apk-android)
7. [Segurança, Antiban e Diretrizes](#7-segurança-antiban-e-diretrizes)
8. [Licença & Termos de Uso](#8-licença--termos-de-uso)

---

## 1. Visão Geral & Arquitetura do Sistema

O **ARDiscordBypass v3.0** foi concebido para contornar instabilidades e bloqueios de infraestrutura de mídia RTC (WebRTC) e transmissões de tela/vídeo do Discord no território brasileiro, sem afetar o tráfego global da máquina e sem necessitar de VPN de sistema integral.

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

O projeto é estruturado em domínios isolados por plataforma:

```
ARDiscordBypass/
├── DOCS.md                     # Esta documentação técnica
├── README.md                   # Apresentação comercial e guia do usuário
├── LICENSE                     # Termos da licença ARGLFU v2
├── logo.png                    # Identidade visual oficial ALRI
├── ARDiscordBypass_linux       # Executável compilado Linux x86_64
├── ARDiscordBypass.exe         # Executável compilado Windows x64
├── ARDiscordBypass.apk         # Aplicativo Android compilado
│
├── PC/                         # Solução Desktop em C++20 Nativo
│   ├── CMakeLists.txt          # Configuração CMake do módulo PC
│   ├── build.sh                # Script de compilação automatizada Linux
│   ├── build.bat               # Script de compilação automatizada Windows
│   ├── main.cpp                # Ponto de entrada, verificador e renderizador CLI
│   ├── ui.h                    # Paleta TrueColor ANSI, Braille spinners e responsividade
│   └── os/
│       ├── linux/
│       │   └── platform.h      # Network Namespaces (`ip netns`), resolv e WireGuard
│       └── windows/
│           └── platform.h      # WireSock WFP, injeção de processos e desanexação de console
│
└── Mobile/                     # Soluções Mobile Nativas
    └── Android/                # App Android (Kotlin, Jetpack, WireGuard GoBackend)
        ├── app/
        │   ├── build.gradle.kts
        │   └── src/main/
        │       ├── AndroidManifest.xml
        │       ├── java/com/alrigroup/ardcb/
        │       │   ├── MainActivity.kt             # Terminal interativo OLED e BottomSheet
        │       │   ├── service/DiscordVpnService.kt# VpnService em primeiro plano
        │       │   ├── tunnel/WireGuardConfigManager.kt # Split-tunneling por UID
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

### 3.2 Windows — Windows Filtering Platform (WFP) via WireSock

No Windows, namespaces de rede não existem no mesmo formato do Linux. A solução de nível profissional é implementada através do **Windows Filtering Platform (WFP)** com motor WireSock:

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
3. **Desanexação de Handles e Limpeza de Console**:
   O processo do Discord e do WireSock são gerados através da Win32 API (`CreateProcessW`) com as flags:
   - `DETACHED_PROCESS`
   - `CREATE_BREAKAWAY_FROM_JOB`
   - Redirecionamento de `hStdOutput` e `hStdError` para `NUL`.
   Isso garante que nenhum log do Discord polua o console do ARDiscordBypass.

### 3.3 Camada de Interface & UX CLI (C++20)

- **Detecção de Dimensão do Terminal**: Realizada via `ioctl(TIOCGWINSZ)` no Linux e `GetConsoleScreenBufferInfo` no Windows.
- **Renderização Adaptativa**: Alterna entre a arte ASCII completa (para telas ≥ 80 colunas) e a arte compactada (para terminais estreitos), prevenindo quebras visuais.
- **Animações de Escrita e Loading**: A logo é construída em camadas com efeitos de pulso de brilho e os spinners operam assincronamente através de `std::async` e `std::atomic<bool>`.

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

### 4.4 Futuro: iOS Engine (NetworkExtension)

A estrutura de pastas `Mobile/` foi projetada para receber o projeto Swift/Xcode utilizando `NETunnelProvider` com motor WireGuardKit para iOS.

---

## 5. Infraestrutura do Túnel & Criptografia

| Parâmetro | Valor / Especificação |
| :--- | :--- |
| **Protocolo** | WireGuard (UDP) |
| **Criptografia Simétrica** | ChaCha20 com autenticação Poly1305 (RFC 7539) |
| **Troca de Chaves** | Curve25519 (ECDH) |
| **Função de Hash** | BLAKE2s |
| **Endpoint do Gateway** | `127.0.0.1:51820` |
| **MTU** | 1280 (Garante passagem sem fragmentação) |
| **Keepalive** | 25 segundos |

---

## 6. Instruções de Compilação & Build

### 6.1 Build Linux nativo

Requisitos: GCC 11+ ou Clang 13+ com suporte a C++20 e CMake 3.16+.

```bash
cd PC
chmod +x build.sh
./build.sh
```

Ou manualmente pelo CMake:
```bash
cd PC
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

### 6.2 Cross-compile MinGW para Windows

No Linux (Arch/Ubuntu/Debian com `mingw-w64` instalado):
```bash
cd PC
x86_64-w64-mingw32-g++ -std=c++20 -O2 \
  -static -static-libgcc -static-libstdc++ \
  -I. \
  main.cpp \
  -o ARDiscordBypass.exe \
  -lws2_32 -lwinhttp -lshell32 -lole32

x86_64-w64-mingw32-strip ARDiscordBypass.exe
```

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

---

## 7. Segurança, Antiban e Diretrizes

1. **Zero Modificação de Binários**: Não há injeção de DLLs, ganchos em memória (*API hooking*) ou modificações no `app.asar` ou no APK do Discord.
2. **Conformidade com os Termos do Discord**: O Discord permite conexões através de redes corporativas, VPNs e túneis proxy. O método utilizado replica um ambiente de rede normal.
3. **Privacidade de Dados**: O túnel não realiza inspeção de pacotes (*Deep Packet Inspection*) e não descriptografa payloads TLS/HTTPS (WebRTC DTLS/SRTP são mantidos ponta a ponta).
4. **Integridade de Binários e Análise Heurística**:
   - Binários Linux e Android possuem **0 detecções** em varreduras antivírus públicas.
   - O binário Windows apresenta alertas heurísticos restritos a motores secundários no VirusTotal devido à compilação estática (`-static`), chamadas de gerenciamento de processos isolados (`CreateProcessW`) e ausência de assinatura de código com certificado pago. Todos os motores líderes (Microsoft Defender, Kaspersky, Bitdefender, ESET, Sophos) atestam o binário como seguro e limpo.
   - As somas de verificação SHA-256 são registradas e auditadas a cada versão lançada.

---

## 8. Licença & Termos de Uso

Este projeto é protegido sob os termos da **ARGLFU (ALRI GROUP LICENSE FREE USE) Version 2 – 2026**.

- Permitido o uso pessoal, gratuito e redistribuição na forma original.
- Proibida a engenharia reversa para fins comerciais ou monetização deste software.

**ALRI GROUP** — *Inovação, Segurança e Performance.*
