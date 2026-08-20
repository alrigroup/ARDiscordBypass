# 🚀 ARDiscordBypass

<p align="center">
  <b>Solução nativa, ultraleve e segura para liberação de Transmissão de Tela (Live / Screen Share) no Discord.</b>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/Language-C%2B%2B20-blue.svg" alt="C++20">
  <img src="https://img.shields.io/badge/Platform-Windows%20%7C%20Win32-0078D6.svg" alt="Windows Win32">
  <img src="https://img.shields.io/badge/Size-~300KB-brightgreen.svg" alt="Size ~300KB">
  <img src="https://img.shields.io/badge/RAM-~2MB-success.svg" alt="RAM ~2MB">
  <img src="https://img.shields.io/badge/TOS-100%25%20Safe-orange.svg" alt="100% TOS Safe">
</p>

---

## 📦 Download Pronto para Uso

Não quer compilar? O executável já está pronto e compilado para uso imediato!

👉 **[Baixe a versão mais recente na aba Releases](https://github.com/alrigroup/ARDicordBypass/releases)**

---

## 📌 Por que usar o ARDiscordBypass?

Muitos usuários enfrentam problemas de restrição ou falhas ao tentar transmitir a tela no Discord no Brasil. As soluções alternativas trazem sérios riscos e perda de desempenho:

1. **VPNs Tradicionais**: Redirecionam **todo o tráfego do seu computador**, causando **lag severo, perda de pacotes e alto ping em jogos**, além de consumirem memória e CPU que causam **queda (drop) de FPS nos jogos**.
2. **Clients Modificados (Vencord, BetterDiscord, Replugged, etc.)**: Injetam código ou alteram arquivos internos do Discord. Além de adicionarem processos extras que **pesam no desempenho**, isso **viola diretamente os Termos de Serviço (TOS) do Discord**, podendo resultar no **banimento permanente da conta**.
3. **Sites de Terceiros de Compartilhamento de Tela**: Simulam a experiência em páginas web pesadas no navegador (consumindo muita RAM/CPU e afetando o FPS do jogo). **Grave risco de privacidade**: os proprietários ou servidores desses sites podem espionar ou visualizar a sua tela e áudio sem o seu consentimento.

---

## ⚡ A Solução Definitiva

O **ARDiscordBypass** resolve esse problema de maneira simples, segura e extremamente otimizada:

* **🎮 Sem Queda de FPS em Jogos**: O executável é **ultraleve** (~300 KB) e **se encerra automaticamente** logo após iniciar o Discord. Ele não fica rodando em segundo plano, liberando 100% da sua CPU e RAM para o seu jogo.
* **🔒 Privacidade & Transmissão Nativa no Discord**: Sua transmissão continua acontecendo 100% dentro do aplicativo oficial do Discord, com a segurança original do app e sem intermediários visualizando sua tela.
* **🎯 Redirecionamento Focado**: Redireciona **apenas** a verificação de localização inicial do aplicativo Discord para fora do Brasil durante a abertura do programa.
* **⚡ Zero Lag de Conexão**: Todo o tráfego do sistema (seus jogos, navegadores, downloads) continua 100% na sua conexão normal de internet.
* **📹 Mídia e Áudio Diretos**: As conexões de voz, vídeo e transmissão (`cdn.discordapp.com`, `*.discord.media`, etc.) utilizam a lista de exceção (`proxy-bypass`), garantindo transmissão em alta velocidade sem intermediários.
* **🛡️ 100% Seguro & Antiban**: **Zero modificação em arquivos do Discord, zero alteração de memória e zero injeção de código**. O bypass utiliza apenas flags nativas suportadas pelo próprio ecossistema Chromium/Electron (`--proxy-server` e `--proxy-bypass-list`).

---

## 📊 Comparativo: ARDiscordBypass vs Outras Soluções

| Recurso / Característica | VPN Global | Client Modificado (Vencord/BetterDiscord) | Sites de Terceiros (Web) | 🚀 **ARDiscordBypass** |
| :--- | :---: | :---: | :---: | :---: |
| **Impacto no FPS dos Jogos** | 🔴 Alto Drop de FPS | ⚠️ Pode causar drop | 🔴 Alto (Uso de RAM do Browser) | 🟢 **ZERO (Programa se encerra após abrir)** |
| **Transmissão Nativa no App do Discord** | ✅ Sim | ✅ Sim | ❌ Não (Via Navegador) | ✅ **Sim** |
| **Privacidade da Sua Tela Garantida** | ✅ Sim | ✅ Sim | 🔴 **NÃO (Risco de espionagem)** | ✅ **Sim (100% Discord Oficial)** |
| **Ping Normal em Jogos** | ❌ Não (Lag) | ✅ Sim | ✅ Sim | ✅ **Sim (0% de impacto)** |
| **Risco de Banimento no Discord** | 🟢 Baixo | 🔴 **ALTO (Viola TOS)** | 🟢 Nulo | 🟢 **ZERO (Parâmetros nativos)** |
| **Sem Injeção de Código DLL/JS** | ✅ Sim | ❌ Não | ✅ Sim | ✅ **Sim (Flags Chromium)** |
| **Uso de Memória RAM** | ⚠️ Alto (100MB+) | ⚠️ Médio | ⚠️ Alto (Navegador) | ⚡ **Irrelevante (< 2MB ao iniciar)** |

---

## ⚙️ Como Funciona Internamente

1. **Detecção Automática**: Identifica a pasta de instalação do Discord no seu sistema (`%LOCALAPPDATA%` ou `Program Files`).
2. **Fechamento Seguro**: Caso o Discord já esteja aberto, encerra as instâncias ativas para aplicar a nova configuração.
3. **Inicialização com Flags Nativas**: Inicializa o processo do Discord utilizando os argumentos nativos de rede da engine Chromium:
   - `--proxy-server`: Redireciona a checagem geográfica de abertura.
   - `--proxy-bypass-list`: Define exceções para que todo o tráfego de mídia, voz e stream passe diretamente pela sua internet normal.
4. **Encerramento Rápido**: O programa encerra imediatamente após iniciar o Discord, deixando a sua máquina 100% livre sem processos pesados rodando em segundo plano.

---

## 📖 Como Usar (Sem precisar compilar)

1. Vá até a aba **[Releases](https://github.com/alrigroup/ARDicordBypass/releases)** do repositório e baixe o arquivo `ARDiscordBypass.exe`.
2. **Execute** o `ARDiscordBypass.exe`.
3. O programa vai detectar a instalação do Discord, fechar instâncias abertas e iniciá-lo com o bypass ativado.
4. **Aproveite!** Sua transmissão de tela no Discord estará liberada sem lag, sem queda de FPS e com total segurança.

> **Dica**: Você pode fixar o atalho do `ARDiscordBypass.exe` na barra de tarefas ou na área de trabalho para abrir o Discord diretamente por ele.

---

## 🛠️ Como Compilar (Opcional)

Se você preferir compilar o código-fonte por conta própria, o projeto é escrito em C++20 nativo para Windows sem dependências externas de terceiros.

### Requisitos:
- Compilador C++20 (GCC/MinGW, MSVC ou Clang)
- Windows SDK (`ws2_32.lib`, `winhttp.lib`, `shell32.lib`)

---

### Opção 1: Usando o Script Automatizado (`build.bat`)
Basta dar dois cliques no arquivo [`build.bat`](file:///c:/Users/WMAROS11U/Documents/ALRIDCBYPASS/build.bat) ou executá-lo pelo Terminal/CMD:
```cmd
build.bat
```

---

### Opção 2: Compilação Manual via GCC / MinGW
```cmd
g++ -O2 -std=c++20 main.cpp -lws2_32 -lwinhttp -lshell32 -o ARDiscordBypass.exe
```

---

### Opção 3: Compilação Manual via MSVC (Visual Studio Developer Command Prompt)
```cmd
cl /O2 /std:c++20 /EHsc main.cpp ws2_32.lib winhttp.lib shell32.lib /Fe:ARDiscordBypass.exe
```

---

### Opção 4: Compilação via CMake
```cmd
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

---

## 🗺️ Roadmap & Planos Futuros

Atualmente, o **ARDiscordBypass** está focado e perfeitamente otimizado para **Windows**. No entanto, temos em mente expandir o suporte nativo no futuro para outras plataformas:

- [x] 🟢 **Windows** (Suporte nativo Win32/C++)
- [ ] 🟡 **Linux** *(Em desenvolvimento/breve)*
- [ ] 🟡 **macOS** *(Em breve)*
- [ ] 🟡 **Android & iOS** *(Planejado para futuras versões)*

---

## 👤 Créditos

- **Desenvolvimento & Autor**: Alexsanderalri.
- **Linguagem**: C++20 / Win32 API.

---

<p align="center">
  <sub><i>Aviso: Este projeto não é afiliado nem endossado pelo Discord Inc. Trata-se de uma ferramenta utilitária que faz uso de parâmetros nativos do executável Chromium.</i></sub>
</p>
