<p align="center">
  <img src="logo.png" alt="ARDiscordBypass Logo" width="180" />
</p>

# 🚀 ARDiscordBypass v3.5.0

<p align="center">
  <b>Solução nativa, isolada por processo e segura para liberação de Transmissão de Tela (Live) e Câmera no Discord no Brasil.</b>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/Language-C%2B%2B20%20%7C%20Kotlin-blue.svg" alt="C++20 Kotlin">
  <img src="https://img.shields.io/badge/Platform-Windows%20%7C%20Linux%20%7C%20Android-E50914.svg" alt="Windows Linux Android">
  <img src="https://img.shields.io/badge/Version-3.5.0-brightgreen.svg" alt="Version 3.5.0">
  <img src="https://img.shields.io/badge/Tunnel-WireGuard%20Isolated-success.svg" alt="WireGuard Isolated">
  <img src="https://img.shields.io/badge/TOS-100%25%20Safe-orange.svg" alt="100% TOS Safe">
  <a href="LICENSE"><img src="https://img.shields.io/badge/License-ARGLFU%20v2-red.svg" alt="License ARGLFU v2"></a>
</p>

---

## ⚡ Novidades da Versão 3.5.0

A **versão 3.5.0** foi completamente reescrita em **C++20 nativo** e **Kotlin**, substituindo o antigo sistema SOCKS5 por um túnel **WireGuard de alta performance isolado por processo (Per-App / Per-Process Split-Tunneling)**:

- 🔒 **Isolamento de Processo**: Apenas o Discord passa pelo túnel. Seus navegadores, jogos, downloads e o restante do sistema mantêm sua conexão de internet original com **0ms de impacto no ping**.
- 🚀 **Executável Único e Autônomo (Single Standalone Executable)**: Tanto no Linux quanto no Windows, o motor gerador Proton vem **embutido como objeto binário no executável**. Zero dependência de pastas auxiliares (`bin/`) ou DLLs externas — basta baixar o `.exe` ou binário Linux e rodar! A extração ocorre de forma transparente e atômica para a pasta de dados locais do usuário (`%LOCALAPPDATA%\ARDiscordBypass` no Windows ou `~/.local/share/ARDiscordBypass` no Linux).
- 🪟 **Execução Segura no Windows (`CreateProcessW` + WFP)**: Integração nativa de baixo nível via driver WFP e chamadas seguras da Win32 API (`CreateProcessW`) com pipes anônimos dedicados (substituindo chamadas frágeis de shell `cmd.exe`/`_popen`), garantindo suporte total a senhas com caracteres especiais (`&`, `%`, `^`), desanexação limpa de processos e streaming de progresso em tempo real sem janelas de console piscando.
- 🐧 **Linux Network Namespaces (`netns`)**: Criação de namespace isolado (`ar-discord-vpn`) no kernel Linux, sem necessidade de alterar a rota padrão global da sua máquina.
- 📱 **Aplicativo Android Nativo**: Split-tunneling por identificador de pacote (`com.discord`), interface inspirada em terminal retro-moderno com fundo 100% OLED Black, Quick Settings Tile nas notificações e Bottom Sheet de notas.
- 🎨 **CLI Responsivo com Animações**: Detecção dinâmica do tamanho do console, construção progressiva da logo ALRI em ASCII e Braille spinners assíncronos.
- 🖥️ **Edição GUI Opcional (Dear ImGui + OpenGL)**: Nova interface gráfica completa com o mesmo design system do app Android: suporte nativo a temas **Dark Mode (100% OLED Pure Black)** e **Light Mode**, renderização da logo oficial da ALRI incorporada na compilação, ícones vetoriais procedurais (zero caracteres corrompidos), animações fluidas de loading (spinner rotativo, reticências dinâmicas, barra de progresso com shimmer e ponto pulsante de status), seletor visual de `.conf`, gerenciador de credenciais Proton e 0% de CPU/GPU em repouso.
- 🔑 **Persistência de Sessão (Até 1 Ano)**: Uma vez autenticado, o certificado criptográfico Curve25519 e a sessão ficam salvos com segurança em `~/.protonvpn-session.json` (`%USERPROFILE%\.protonvpn-session.json`), permitindo conexões em **1 clique instantâneo** nas próximas execuções.

---

## 🖥️ Duas Edições Disponíveis: CLI vs GUI

O **ARDiscordBypass** é construído sobre um núcleo C++ unificado e distribuído em duas edições sob medida:

| Característica | ⚡ Edição CLI (Ultra-leve) | 🖥️ Edição GUI (Interface Gráfica) |
| :--- | :---: | :---: |
| **Arquivos Compilados** | `ARDiscordBypass_linux`<br>`ARDiscordBypass.exe` | `ARDiscordBypass_gui_linux`<br>`ARDiscordBypass_gui.exe` |
| **Tamanho do Binário** | ~13 MB (Tudo-em-um) | ~14 MB (Tudo-em-um com Dear ImGui) |
| **Arquitetura Portátil** | **100% Arquivo Único (Standalone)**<br>*Gerador Proton embutido e auto-extraível* | **100% Arquivo Único (Standalone)**<br>*Gerador Proton e Logo OpenGL embutidos* |
| **Experiência Visual** | Terminal animado TrueColor (Braille dots) | Janela moderna OLED Dark (Dear ImGui) |
| **Ideal para** | Inicialização silenciosa no boot, atalhos, scripts | Uso diário, monitoramento de ping/IP e alternância de perfis |
| **Controles** | Flags (`-stop`, `-cfg`, `-l`, `-h`) | Botão de alternância, seletor de arquivo nativo e tabs |
| **Consumo em Repouso** | 0% CPU / 0% GPU | 0% CPU / 0% GPU (suspensão de renderização ociosa) |

---

## 📦 Downloads & Verificação de Integridade (Hashes & VirusTotal)

Para total transparência e segurança de todos os usuários, disponibilizamos os relatórios públicos do **VirusTotal** e as hashes **SHA-256** oficiais de cada binário compilado. Você pode conferir a integridade do seu arquivo executando `sha256sum <arquivo>` no Linux ou `CertUtil -hashfile <arquivo> SHA256` no Windows:

| Plataforma | Edição | Arquivo | Status VirusTotal | Hash SHA-256 Oficial |
| :--- | :---: | :--- | :---: | :--- |
| **Android** | Mobile | `ARDiscordBypass.apk` | [**0/75 Detections (100% Limpo)**](https://www.virustotal.com/gui/file/3b4891d8e91de6689b1523c59e2aa8f291e8b40363c69efdb89a70668682db2c?nocache=1) | `3b4891d8e91de6689b1523c59e2aa8f291e8b40363c69efdb89a70668682db2c` |
| **Linux (x86_64)** | CLI | `ARDiscordBypass_linux` | [**0/75 Detections (100% Limpo)**](https://www.virustotal.com/gui/file/c1675e9cbadda07ba2e7535981bd80e942bfd0c46796fcd35d980dd2860761f1?nocache=1) | `c1675e9cbadda07ba2e7535981bd80e942bfd0c46796fcd35d980dd2860761f1` |
| **Linux (x86_64)** | GUI | `ARDiscordBypass_gui_linux` | [**0/75 Detections (100% Limpo)**](https://www.virustotal.com/gui/file/6e6cb8b1e5815ef4faefd02a5b75502fa85ca81e0b81d3c480e2949bf3bfea8e?nocache=1) | `6e6cb8b1e5815ef4faefd02a5b75502fa85ca81e0b81d3c480e2949bf3bfea8e` |
| **Windows (x64)** | CLI | `ARDiscordBypass.exe` | [**4/75 Detections (Falso Positivo)**](https://www.virustotal.com/gui/file/df9983198147fb850b473ecd58f84fac72059eb9e75c54e31801c25ed1c7cecd?nocache=1) | `df9983198147fb850b473ecd58f84fac72059eb9e75c54e31801c25ed1c7cecd` |
| **Windows (x64)** | GUI | `ARDiscordBypass_gui.exe` | [**3/75 Detections (Falso Positivo)**](https://www.virustotal.com/gui/file/0907e8d7e9ea1927fc6e5cfd35b59169d2cafe23f3a60832430031690e4468fa?nocache=1) | `0907e8d7e9ea1927fc6e5cfd35b59169d2cafe23f3a60832430031690e4468fa` |

> ℹ️ **Por que os binários do Windows apontam algumas detecções heurísticas (Falso Positivo)?**
> 
> O código do **ARDiscordBypass é 100% aberto e auditável**. Esse comportamento no Windows é um **falso positivo clássico de antivírus de segunda linha** pelos seguintes motivos técnicos:
> 1. **Linkagem Estática do MinGW (`-static`)**: Para que você não precise instalar DLLs ou o Microsoft Visual C++ Redistributable, o binário embute as bibliotecas C++ padrão diretamente dentro do `.exe`, o que alguns motores heurísticos marcam preventivamente.
> 2. **Manipulação de Processos e Rede (Win32 API)**: O executável utiliza APIs do sistema operacional (`CreateProcessW` com `DETACHED_PROCESS` e manipulação de handles de rede) para gerenciar o túnel de split-tunneling e lançar o Discord em background sem vazar logs no console. Antivírus genéricos costumam alertar sobre programas novos/sem assinatura digital que iniciam outros softwares.
> 3. **Ausência de Certificado Comercial Pago**: O executável é de código aberto e não possui um certificado Code Signing comercial (que custa centenas de dólares anuais). Os principais antivírus do mercado (Microsoft Defender, Kaspersky, Bitdefender, ESET, Sophos, Avast) reconhecem o binário como **100% seguro e não malicioso**.
> 4. **Compilação Independente**: Se desejar, você mesmo pode inspecionar o código em `PC/main.cpp` e compilá-lo com seu próprio compilador executando `build.bat`!

---

## 🔑 Como Obter Sua Conexão WireGuard Gratuita (ProtonVPN)

Para garantir **estabilidade máxima**, **zero quedas de conexão** e **privacidade total**, o **ARDiscordBypass v3.5.0** não utiliza chaves públicas compartilhadas (que causam colisões e bloqueios de pacotes quando múltiplos usuários tentam transmitir ao mesmo tempo). Em vez disso, o aplicativo gera ou utiliza um túnel WireGuard exclusivo para cada usuário.

### 1. Crie uma Conta Gratuita no ProtonVPN
> 💡 **AVISO IMPORTANTE: É 100% GRATUITO e você NÃO precisa pagar nada nem cadastrar cartão de crédito!**
> 
> A ProtonVPN disponibiliza um plano gratuito ilimitado oficial. Para criar:

1. Acesse o site oficial: [**https://account.protonvpn.com/pt-br/login**](https://account.protonvpn.com/pt-br/login)
2. Selecione a opção **"Get Proton Free"** (ou "Continuar com o plano gratuito").
3. Crie um nome de usuário, informe seu email e defina uma senha.
4. Conclua a verificação gratuita por email/código e sua conta estará ativa.

---

### 2. Como Usar no ARDiscordBypass

#### 💻 No PC (Linux e Windows):
- **100% Arquivo Único e Portátil (Single Standalone Executable)**: Você não precisa baixar nenhuma pasta `bin/`, arquivo auxiliar ou DLL. O motor do Proton e a logo gráfica são **embutidos diretamente dentro do executável compilado**. Basta dar dois cliques no arquivo `.exe` (ou executar no Linux) em qualquer pasta (ex: Downloads ou Área de Trabalho). Na primeira vez, o próprio programa auto-extrai o utilitário Proton para os dados locais protegidos do usuário (`%LOCALAPPDATA%\ARDiscordBypass` no Windows ou `~/.local/share/ARDiscordBypass` no Linux).
- **Primeira Execução**: Ao abrir o `./ARDiscordBypass_linux` ou `ARDiscordBypass.exe` (ou pela interface gráfica), informe seu **Usuário** (ex: `usuario@proton.me` ou `seu_email@gmail.com`) e **Senha** apenas uma única vez.
- **Automação Completa via Win32 API / Kernel Linux**: O aplicativo executa o handshake SRP com a API do Proton em segundo plano (usando `CreateProcessW` nativo com pipes no Windows e isolamento seguro no Linux, sem abrir janelas extras do CMD), seleciona o servidor internacional gratuito com menor carga e gera o perfil de chaves WireGuard exclusivo `ardc_vpn.conf`.
- **Próximas Execuções**: A sessão e as chaves permanecem salvas em `~/.protonvpn-session.json` (`%USERPROFILE%\.protonvpn-session.json` no Windows), válidas por até 1 ano. Nas próximas vezes, o bypass conecta **em 1 clique instantâneo** sem pedir credenciais!
- **Instalação Automática de Dependências (Linux)**: O aplicativo detecta sua distribuição (`Arch`, `Ubuntu`, `Debian`, `Fedora`, `openSUSE`) e instala silenciosamente utilitários essenciais ausentes (`wireguard-tools`, `iproute2`, `iptables`, `curl`) sem exigir comandos manuais.

#### 📱 No Android:
- O aplicativo Android conta com um sistema moderno de **Abas por Categoria** idêntico à versão Desktop GUI:
  - **DASHBOARD**: Monitoramento em tempo real com Badge de status ("PROTEGIDO" / "DESCONECTADO"), métricas da interface `tun0`, roteamento isolado por processo, latência (ping dinâmico), botão central de ativação 100% ALRI Red e atalho de inicialização direta do Discord.
  - **CONFIGURAÇÕES**: Gerenciamento de perfil WireGuard com botão de importação (`.conf`) e seção exclusiva de **Limpeza Total de Configurações** para apagar perfis locais e resetar dados com 1 clique.
  - **LOGS**: Console terminal retro-moderno com rolagem automática e botão de limpeza de logs.
  - **SOBRE**: Informações da versão, licença ARGLFU v2, créditos e atalhos para documentação técnica e repositório.

#### 🧹 Como Funciona a Limpeza Total de Configurações e Sessões?
Se você desejar apagar o perfil salvo, remover credenciais de login ou restaurar o aplicativo ao estado original de fábrica:
- **No PC (GUI)**: Acesse a aba **CONFIGURAÇÕES** e clique no botão **"LIMPAR TODAS AS CONFIGURAÇÕES E DADOS SALVOS"**. O aplicativo encerra qualquer túnel ativo, exclui `ardc_vpn.conf` e apaga a sessão Proton salva em `~/.protonvpn-session.json` (`%USERPROFILE%\.protonvpn-session.json` no Windows).
- **No PC (CLI)**: Execute o comando `./ARDiscordBypass_linux -stop` e remova os arquivos locais de perfil e sessão.
- **No Android (APK)**: Acesse a aba **CONFIGURAÇÕES** e toque no botão **"LIMPAR TODAS AS CONFIGURAÇÕES E DADOS"**. O aplicativo desativa imediatamente a VPN, deleta o arquivo `ardc_vpn.conf` do armazenamento privado do app e limpa o buffer de logs.

#### ⌨️ CLI Embutido na Versão GUI (Desktop):
A versão GUI (`ARDiscordBypass_gui_linux` e `ARDiscordBypass_gui.exe`) **contém a CLI totalmente embutida**. Se executada com parâmetros pelo terminal ou CMD, ela roda instantaneamente em modo headless/console sem abrir a interface gráfica:

| Parâmetro | Descrição |
| :--- | :--- |
| `(sem argumentos)` | Inicia a interface gráfica moderna Dear ImGui (ou o assistente CLI caso não haja servidor gráfico). |
| `--cli` | Força a execução puramente em modo de linha de comando no terminal. |
| `-stop`, `--stop` | **Encerramento total**: Fecha todas as instâncias do Discord, destrói o túnel WireGuard e remove o namespace isolado / WireSock, devolvendo a rede ao estado padrão original. |
| `-cfg <caminho>`, `--config` | **Configuração personalizada**: Aponta diretamente para qualquer arquivo `.conf` WireGuard no seu disco sem precisar renomear. |
| `-l`, `--login`, `--proton` | **Renovação de credenciais**: Força um novo login na API do ProtonVPN para gerar chaves e túnel atualizados. |
| `-h`, `--help` | Exibe o menu de ajuda com todos os parâmetros disponíveis. |

**Exemplos:**
```bash
# Desfazer o bypass e fechar tudo:
./ARDiscordBypass_linux -stop

# Conectar usando seu próprio arquivo .conf:
./ARDiscordBypass_linux -cfg ~/Downloads/minha_vpn.conf

# No Windows (Prompt de Comando ou PowerShell):
ARDiscordBypass.exe -stop
ARDiscordBypass.exe -cfg "C:\Users\Nome\Downloads\minha_vpn.conf"
```

---

## 📊 Comparativo: ARDiscordBypass vs Outras Soluções

| Recurso / Característica | VPN Global | Client Modificado (Vencord/BetterDiscord) | Sites de Terceiros (Web) | 🚀 **ARDiscordBypass v3.5.0** |
| :--- | :---: | :---: | :---: | :---: |
| **Impacto no FPS dos Jogos** | 🔴 Alto Drop de FPS | ⚠️ Pode causar drop | 🔴 Alto (Uso de RAM do Browser) | 🟢 **ZERO (Isolado no Discord)** |
| **Transmissão Nativa no App** | ✅ Sim | ✅ Sim | ❌ Não (Via Navegador) | ✅ **Sim (100% Oficial)** |
| **Privacidade da Sua Tela** | ✅ Sim | ✅ Sim | 🔴 NÃO (Risco de monitoramento) | 🟢 **100% Protegido (P2P)** |
| **Segurança da Conta (TOS)** | ✅ Seguro | 🔴 **Risco de Ban permanente** | ⚠️ Questionável | 🟢 **100% Seguro & Antiban** |
| **Latência (Ping) nos Jogos** | 🔴 Ping Alto (+100ms) | 🟢 Ping Normal | 🟢 Ping Normal | 🟢 **Ping Nativo (Intocado)** |
| **Configuração Manual** | 🔴 Exige contas e apps pesados | ⚠️ Exige patch em arquivos | ⚠️ Exige login em sites | 🟢 **Zero Configuração (1 Clique)** |

---

## 📂 Organização do Repositório

```
ARDiscordBypass/
├── PC/
│   ├── main.cpp              ← Ponto de entrada C++20 (responsividade e animações)
│   ├── ui.h                  ← Engine de UI terminal, Braille dots e TrueColor
│   ├── CMakeLists.txt        ← CMake do módulo Desktop
│   ├── build.sh              ← Script de automação Linux (CLI + GUI)
│   ├── build.bat             ← Script de automação Windows (CLI + GUI)
│   ├── core/service.h        ← Gerenciador de túnel e processos do Desktop
│   ├── gui/                  ← Interface gráfica OpenGL Dear ImGui (Dark/Light)
│   └── os/
│       ├── linux/platform.h  ← Namespace WireGuard isolado (`ar-discord-vpn`)
│       └── windows/platform.h← WireSock WFP e controle de processos Win32
│
├── tools/
│   └── proton-confgen/       ← Código-fonte aberto do gerador de perfil Proton (Go 1.22+)
│       ├── Makefile          ← Script de compilação multi-plataforma
│       ├── cmd/protonvpn-wg/ ← Ponto de entrada CLI (main.go com DNS fallback)
│       └── internal/         ← SRP auth, uTLS client, API Proton e WireGuard keygen
│
├── Mobile/
│   ├── Android/              ← App Android nativo (Kotlin, VpnService, WireGuard GoBackend)
│   │   └── app/src/main/jniLibs/arm64-v8a/
│   │       └── libprotonconfgen.so ← Binário nativo gerado a partir de tools/proton-confgen
│   └── (iOS)/                ← Planejado (NetworkExtension)
│
├── DOCS.md                   ← Documentação técnica completa e engenharia de rede
├── README.md                 ← Guia geral do projeto
├── ARDiscordBypass_linux     ← Binário CLI compilado Linux (~13 MB tudo-em-um)
├── ARDiscordBypass_gui_linux ← Binário GUI Desktop Linux com Dear ImGui (~14 MB tudo-em-um)
├── ARDiscordBypass.exe       ← Binário CLI compilado Windows (~13 MB tudo-em-um)
├── ARDiscordBypass_gui.exe   ← Binário GUI Desktop Windows com Dear ImGui (~14 MB tudo-em-um)
└── ARDiscordBypass.apk       ← Aplicativo Android com abas por categorias (~16 MB)
```

---

## 🔓 Transparência Open-Source: Sobre a Biblioteca `libprotonconfgen.so`

> ⚠️ **AVISO DE TRANSPARÊNCIA TOTAL: O ARDiscordBypass NÃO possui nenhuma biblioteca fechada, proprietária ou blob binário opaco.**

Muitos projetos dizem ser código aberto mas embutem binários compilados sem fornecer o código-fonte correspondente. No **ARDiscordBypass**, prezamos pela honestidade técnica e integridade máxima:

1. **Onde está o código-fonte de `libprotonconfgen.so`?**
   O código-fonte completo está disponível na pasta **[`tools/proton-confgen/`](tools/proton-confgen/)** deste repositório, escrito em **Go puro** e licenciado abertamente.
2. **Por que o arquivo termina com `.so` e fica em `jniLibs`?**
   No Android moderno (Android 10+ / 14+), as diretrizes do sistema operacional aplicam **W^X** (*Write XOR Execute*) e políticas do **SELinux**:
   - É proibido executar binários extraídos dentro de pastas privadas (`/data/data/...`) sem permissão de Root (`EACCES: Permission denied`).
   - A única forma nativa e segura recomendada pela documentação do Android para executar um binário compilado de apoio é empacotá-lo na pasta `jniLibs/<abi>/` com a nomenclatura de biblioteca do sistema (`lib*.so`) e `useLegacyPackaging = true`.
   - Ao instalar o APK, o próprio sistema operacional Android extrai o arquivo para a partição protegida de bibliotecas `/data/app/.../lib/arm64/` com **permissões reais de execução (`+x`)**, permitindo que o app faça o login e gere o perfil WireGuard de forma segura e autônoma.
3. **Como recompilar o `.so` você mesmo a partir do código Go:**
   ```bash
   cd tools/proton-confgen
   CGO_ENABLED=0 GOOS=android GOARCH=arm64 go build -ldflags "-s -w" -o libprotonconfgen.so cmd/protonvpn-wg/main.go
   cp libprotonconfgen.so ../../Mobile/Android/app/src/main/jniLibs/arm64-v8a/libprotonconfgen.so
   ```
   Qualquer pessoa pode auditar o código Go, recompilar e verificar a exata correspondência das instruções e chamadas de rede.

---

## 🛠️ Como Compilar

Para informações detalhadas sobre arquitetura, protocolos e compilação manual passo a passo em todas as plataformas, consulte o [**DOCS.md**](DOCS.md).

### Linux (Nativo)
```bash
cd PC
chmod +x build.sh
./build.sh
```

### Windows (MinGW / CMD)
Execute `build.bat` dentro da pasta `PC/` para compilação automatizada com o motor embutido:
```cmd
cd PC
build.bat
:: Ou para compilar também a GUI:
build.bat --gui
```
*(Para compilação manual detalhada via MinGW/GCC com `objcopy`, consulte a Seção 6.2 do [**DOCS.md**](DOCS.md)).*

### Android (APK & Motor Go)
```bash
# 1. (Opcional) Recompilar a biblioteca nativa Go do Proton:
cd tools/proton-confgen
CGO_ENABLED=0 GOOS=android GOARCH=arm64 go build -ldflags "-s -w" -o libprotonconfgen.so cmd/protonvpn-wg/main.go
cp libprotonconfgen.so ../../Mobile/Android/app/src/main/jniLibs/arm64-v8a/libprotonconfgen.so
cd ../..

# 2. Compilar o aplicativo Android com Gradle:
cd Mobile/Android
./gradlew assembleDebug
# Gera o arquivo: app/build/outputs/apk/debug/app-debug.apk
```

---

## 🛡️ Segurança & Antiban

O **ARDiscordBypass** opera em conformidade estrita com a segurança do usuário:
- **Zero Injeção**: Não há injeção de código, DLL hijacking ou hooking em memória.
- **Zero Modificação**: Nenhum arquivo do Discord (`app.asar`, recursos ou instalador) é modificado.
- O bypass atua exclusivamente na camada de transporte de rede, criando uma rota dedicada para os pacotes de mídia do Discord através de criptografia padrão da indústria (ChaCha20-Poly1305).

---

## 🤝 Contribuições

Contribuições são bem-vindas! Sinta-se à vontade para abrir uma [Issue](https://github.com/alrigroup/ARDiscordBypass/issues) ou enviar um [Pull Request](https://github.com/alrigroup/ARDiscordBypass/pulls).

---

## 📄 Licença

Este projeto é protegido sob os termos da licença **ARGLFU (ALRI GROUP LICENSE FREE USE) Version 2 – 2026**.

- 🟢 **Uso e Distribuição**: Gratuito para uso pessoal e não comercial em sua forma original.
- 🔴 **Proibição de Modificação / Comercialização**: Vedada a venda, alteração sem autorização ou exploração comercial deste código.

Consulte o arquivo [LICENSE](LICENSE) para maiores informações.

<br />

<p align="center">
  <b>ALRI GROUP</b> — <i>Technology & Software Architecture</i><br>
  Copyright © 2020-2026 ALRI Group. Todos os direitos reservados.
</p>
