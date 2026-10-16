<p align="center">
  <img src="logo.png" alt="ARDiscordBypass Logo" width="180" />
</p>

# 🚀 ARDiscordBypass v3.0

<p align="center">
  <b>Solução nativa, isolada por processo e segura para liberação de Transmissão de Tela (Live) e Câmera no Discord no Brasil.</b>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/Language-C%2B%2B20%20%7C%20Kotlin-blue.svg" alt="C++20 Kotlin">
  <img src="https://img.shields.io/badge/Platform-Windows%20%7C%20Linux%20%7C%20Android-E50914.svg" alt="Windows Linux Android">
  <img src="https://img.shields.io/badge/Version-3.0.0-brightgreen.svg" alt="Version 3.0.0">
  <img src="https://img.shields.io/badge/Tunnel-WireGuard%20Isolated-success.svg" alt="WireGuard Isolated">
  <img src="https://img.shields.io/badge/TOS-100%25%20Safe-orange.svg" alt="100% TOS Safe">
  <a href="LICENSE"><img src="https://img.shields.io/badge/License-ARGLFU%20v2-red.svg" alt="License ARGLFU v2"></a>
</p>

---

## ⚡ Novidades da Versão 3.0.0

A **versão 3.0** foi completamente reescrita em **C++20 nativo** e **Kotlin**, substituindo o antigo sistema SOCKS5 por um túnel **WireGuard de alta performance isolado por processo (Per-App / Per-Process Split-Tunneling)**:

- 🔒 **Isolamento de Processo**: Apenas o Discord passa pelo túnel. Seus navegadores, jogos, downloads e o restante do sistema mantêm sua conexão de internet original com **0ms de impacto no ping**.
- 🐧 **Linux Network Namespaces (`netns`)**: Criação de namespace isolado (`ar-discord-vpn`) no kernel Linux, sem necessidade de alterar a rota padrão global da sua máquina.
- 🪟 **Windows Filtering Platform (WFP)**: Integração nativa de baixo nível via driver WFP com desanexação de console e zero vazamento de logs.
- 📱 **Aplicativo Android Nativo**: Split-tunneling por identificador de pacote (`com.discord`), interface inspirada em terminal retro-moderno com fundo 100% OLED Black, Quick Settings Tile nas notificações e Bottom Sheet de notas.
- 🎨 **CLI Responsivo com Animações**: Detecção dinâmica do tamanho do console, construção progressiva da logo ALRI em ASCII e Braille spinners assíncronos.

---

## 📦 Downloads & Verificação de Integridade (Hashes & VirusTotal)

Para total transparência e segurança de todos os usuários, disponibilizamos os relatórios públicos do **VirusTotal** e as hashes **SHA-256** oficiais de cada binário compilado. Você pode conferir a integridade do seu arquivo executando `sha256sum <arquivo>` no Linux ou `CertUtil -hashfile <arquivo> SHA256` no Windows:

| Plataforma | Arquivo | Status VirusTotal | Hash SHA-256 Oficial |
| :--- | :--- | :---: | :--- |
| **Android** | `ARDiscordBypass.apk` | [**0/65 Detections (100% Limpo)**](https://www.virustotal.com/gui/file/0b92c34efe6baded27665500fea7320a2a7fe8e0c6cf9bdd3d02f3281b0efc40?nocache=1) | `0b92c34efe6baded27665500fea7320a2a7fe8e0c6cf9bdd3d02f3281b0efc40` |
| **Linux (x86_64)** | `ARDiscordBypass_linux` | [**0/63 Detections (100% Limpo)**](https://www.virustotal.com/gui/file/8a788e6a0c39b555b142eb17a4c133b63efba60d1985054c29dbb1309d8b8024?nocache=1) | `8a788e6a0c39b555b142eb17a4c133b63efba60d1985054c29dbb1309d8b8024` |
| **Windows (x64)** | `ARDiscordBypass.exe` | [**5/71 Detections (Falso Positivo)**](https://www.virustotal.com/gui/file/75a5f1f041f1e40ff057d8e26c65d2dc7b02420e91a7633bcc8304aab5197e09?nocache=1) | `75a5f1f041f1e40ff057d8e26c65d2dc7b02420e91a7633bcc8304aab5197e09` |

> ℹ️ **Por que o binário do Windows aponta 5 detecções heurísticas (Falso Positivo)?**
> 
> O código do **ARDiscordBypass é 100% aberto e auditável**. Esse comportamento no Windows é um **falso positivo clássico de antivírus de segunda linha** pelos seguintes motivos técnicos:
> 1. **Linkagem Estática do MinGW (`-static`)**: Para que você não precise instalar DLLs ou o Microsoft Visual C++ Redistributable, o binário embute as bibliotecas C++ padrão diretamente dentro do `.exe`, o que alguns motores heurísticos marcam preventivamente.
> 2. **Manipulação de Processos e Rede (Win32 API)**: O executável utiliza APIs do sistema operacional (`CreateProcessW` com `DETACHED_PROCESS` e manipulação de handles de rede) para gerenciar o túnel de split-tunneling e lançar o Discord em background sem vazar logs no console. Antivírus genéricos costumam alertar sobre programas novos/sem assinatura digital que iniciam outros softwares.
> 3. **Ausência de Certificado Comercial Pago**: O executável é de código aberto e não possui um certificado Code Signing comercial (que custa centenas de dólares anuais). Os principais antivírus do mercado (Microsoft Defender, Kaspersky, Bitdefender, ESET, Sophos, Avast) reconhecem o binário como **100% seguro e não malicioso**.
> 4. **Compilação Independente**: Se desejar, você mesmo pode inspecionar o código em `PC/main.cpp` e compilá-lo com seu próprio compilador executando `build.bat`!

---

## 📊 Comparativo: ARDiscordBypass vs Outras Soluções

| Recurso / Característica | VPN Global | Client Modificado (Vencord/BetterDiscord) | Sites de Terceiros (Web) | 🚀 **ARDiscordBypass v3.0** |
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
│   ├── build.sh              ← Script de automação Linux
│   ├── build.bat             ← Script de automação Windows
│   └── os/
│       ├── linux/platform.h  ← Namespace WireGuard isolado (`ar-discord-vpn`)
│       └── windows/platform.h← WireSock WFP e controle de processos Win32
│
├── Mobile/
│   ├── Android/              ← App Android nativo (Kotlin, VpnService, WireGuard GoBackend)
│   └── (iOS)/                ← Planejado (NetworkExtension)
│
├── DOCS.md                   ← Documentação técnica e arquitetura de rede
├── README.md                 ← Guia geral do projeto
├── ARDiscordBypass_linux     ← Binário compilado Linux
├── ARDiscordBypass.exe       ← Binário compilado Windows
└── ARDiscordBypass.apk       ← Aplicativo Android compilado
```

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
Execute `build.bat` dentro da pasta `PC/` ou utilize o GCC para linkagem estática:
```cmd
g++ -O2 -std=c++20 -static main.cpp -lws2_32 -lwinhttp -lshell32 -lole32 -o ARDiscordBypass.exe
```

### Android (Gradle)
```bash
cd Mobile/Android
./gradlew assembleDebug
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
