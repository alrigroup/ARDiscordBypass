# 🚀 ARDiscordBypass v2.0

<p align="center">
  <b>Solução nativa, ultraleve e segura para liberação de Transmissão de Tela (Live / Screen Share) no Discord.</b>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/Language-C%2B%2B20-blue.svg" alt="C++20">
  <img src="https://img.shields.io/badge/Platform-Windows%20%7C%20Linux-0078D6.svg" alt="Windows Linux">
  <img src="https://img.shields.io/badge/Version-2.0.0-brightgreen.svg" alt="Version 2.0">
  <img src="https://img.shields.io/badge/Status-Correção%20em%20Breve-yellow.svg" alt="Status">
  <img src="https://img.shields.io/badge/Size-~300KB-brightgreen.svg" alt="Size ~300KB">
  <img src="https://img.shields.io/badge/RAM-~2MB-success.svg" alt="RAM ~2MB">
  <img src="https://img.shields.io/badge/TOS-100%25%20Safe-orange.svg" alt="100% TOS Safe">
  <a href="LICENSE"><img src="https://img.shields.io/badge/License-ARGLFU%20v2-red.svg" alt="License ARGLFU v2"></a>
</p>

---

> [!WARNING]
> ### ⚠️ Status Atual: Correção Temporária do Discord
> Recentemente o Discord implementou uma atualização que neutralizou temporariamente este método de bypass.
> **Estamos cientes e já trabalhando nisso:** uma nova versão corrigida será lançada em breve! Acompanhe as atualizações na aba de [Releases](https://github.com/alrigroup/ARDiscordBypass/releases).

---

## 📦 Download Pronto para Uso

Não quer compilar? O executável já está pronto e compilado para uso imediato!

👉 **[Baixe a versão mais recente na aba Releases](https://github.com/alrigroup/ARDiscordBypass/releases)**

---

## ⚠️ Compatibilidade (v2.0)

| Sistema Operacional | Método de Instalação do Discord | Status |
| :--- | :---: | :---: |
| **Windows 11** | Instalador oficial (.exe) | ⚠️ Afetado por atualização do Discord *(Correção em breve)* |
| **Windows 10** | Instalador oficial (.exe) | ⚠️ Afetado por atualização do Discord *(Correção em breve)* |
| **Linux** | Flatpak | ⚠️ Afetado por atualização do Discord *(Correção em breve)* |
| **Linux** | Snap / Nativo (.deb, .tar.gz, AUR, etc.) | ⚠️ Afetado por atualização do Discord *(Correção em breve)* |
| **Linux** | Qualquer outro método | ⚠️ Afetado por atualização do Discord *(Correção em breve)* |

> **Não encontrou seu sistema na lista?** O programa foi feito para funcionar em qualquer distro Linux e qualquer método de instalação do Discord. Se encontrar algum problema, por favor [abra uma Issue](https://github.com/alrigroup/ARDiscordBypass/issues)!

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
* **🌐 Proxy Automático (v2.0)**: Busca automaticamente um proxy SOCKS5 funcional na internet e testa antes de usar. Não precisa configurar nada manualmente!
* **🔄 Verificação de Atualizações (v2.0)**: Verifica automaticamente se há uma versão mais recente disponível no GitHub.

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
| **Suporte a Linux** | ✅ Sim | ⚠️ Limitado | ✅ Sim | ✅ **Sim (Flatpak, Snap, Nativo)** |

---

## ⚙️ Como Funciona Internamente

### Windows
1. **Detecção Automática**: Identifica a pasta de instalação do Discord (`%LOCALAPPDATA%`, `Program Files`).
2. **Fechamento Seguro**: Caso o Discord já esteja aberto, encerra as instâncias ativas para aplicar a nova configuração.
3. **Busca de Proxy**: Consulta a API do ProxyScrape para encontrar um proxy SOCKS5 funcional.
4. **Inicialização com Flags Nativas**: Inicializa o processo do Discord com `--proxy-server` e `--proxy-bypass-list`.
5. **Encerramento Rápido**: O programa se encerra imediatamente após iniciar o Discord.

### Linux
1. **Detecção Automática**: Busca o Discord em Flatpak, Snap, XDG, `/opt`, `/usr/bin` e via `which`.
2. **Fechamento Seguro**: Encerra instâncias ativas via `pkill`.
3. **Busca de Proxy**: Consulta a API do ProxyScrape para encontrar um proxy SOCKS5 funcional.
4. **Inicialização com Flags Nativas**:
   - **Flatpak/Snap**: Cria o arquivo `discord-flags.conf` que o Electron lê automaticamente.
   - **Nativo**: Passa `--proxy-server` e `--proxy-bypass-list` diretamente no executável.
5. **Limpeza**: Remove o arquivo `discord-flags.conf` após o Discord iniciar.

---

## 📖 Como Usar (Sem precisar compilar)

### Windows
1. Vá até a aba **[Releases](https://github.com/alrigroup/ARDiscordBypass/releases)** e baixe o `ARDiscordBypass.exe`.
2. **Execute** o `ARDiscordBypass.exe`.
3. O programa detecta o Discord, fecha instâncias abertas, busca um proxy funcional e inicia o bypass.
4. **Aproveite!** Sua transmissão de tela estará liberada.

### Linux
1. Vá até a aba **[Releases](https://github.com/alrigroup/ARDiscordBypass/releases)** e baixe o `ARDiscordBypass_linux`.
2. Torne executável: `chmod +x ARDiscordBypass_linux`
3. **Execute**: `./ARDiscordBypass_linux`
4. O programa detecta automaticamente o tipo de instalação (Flatpak, Snap ou nativo) e aplica o bypass.

> **Dica**: No Linux, você pode passar o caminho do Discord manualmente: `./ARDiscordBypass_linux /caminho/para/Discord`

---

## ❓ Solução de Problemas

### Discord fica em "loading infinito" (tela de loading por mais de 1 minuto)
Isso acontece quando o proxy selecionado não está respondendo corretamente. A solução é simples:
1. **Feche o Discord** (pelo gerenciador de tarefas se necessário).
2. **Execute o ARDiscordBypass novamente.** O programa buscará um **novo proxy funcional** e o problema será resolvido.

> **Nota**: O programa pode demorar alguns segundos a mais para iniciar — isso é normal, pois a busca por proxies livres depende da disponibilidade de servidores no momento. Se nenhum proxy for encontrado, o Discord abre sem bypass (sem proxy). Tente novamente mais tarde caso o loading infinito persista.

---

## 🛠️ Como Compilar (Opcional)

Se você preferir compilar o código-fonte por conta própria, o projeto é escrito em C++20 nativo para Windows e Linux, sem dependências externas de terceiros.

### Requisitos:
- Compilador C++20 (GCC/MinGW, MSVC ou Clang)
- CMake 3.16 ou superior

### Estrutura do Projeto
```
ARDiscordBypass/
├── main.cpp              ← Lógica principal (versão, update, fluxo)
├── os/
│   ├── linux/
│   │   └── platform.h    ← Tudo específico do Linux
│   └── windows/
│       └── platform.h    ← Tudo específico do Windows
├── CMakeLists.txt
├── build.sh              ← Script de build para Linux
└── build.bat             ← Script de build para Windows
```

### Linux
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/ARDiscordBypass_cpp
```

Ou use o script automatizado:
```bash
chmod +x build.sh
./build.sh
```

### Windows
Basta dar dois cliques no `build.bat` ou executá-lo pelo Terminal/CMD.

Ou compile manualmente:
```cmd
g++ -O2 -std=c++20 -static main.cpp -lws2_32 -lwinhttp -lshell32 -o ARDiscordBypass.exe
```

> **Nota**: O flag `-static` embute todas as DLLs no executável, tornando-o independente. Funciona em qualquer Windows sem instalar nada.

> **Desenvolvedor?** Quer entender o funcionamento avançado do programa, a arquitetura completa, como o proxy automático funciona ou como contribuir com melhorias? Veja o [**DEVELOPER.md**](DEVELOPER.md)!

---

## 🗺️ Roadmap & Planos Futuros

- [x] 🟢 **Windows 10/11** (Suporte nativo Win32/C++)
- [x] 🟢 **Linux** (Suporte Flatpak, Snap e Nativo)
- [ ] 🟡 **Nova versão com correção para atualização do Discord** *(Em andamento)*
- [ ] 🟡 **Android** *(Em breve - em desenvolvimento)*
- [ ] 🔵 **macOS** *(Planos futuros)*
- [ ] 🔵 **iOS** *(Planos futuros)*

---

## 🐛 Encontrou um Bug?

Se você encontrou algum problema, por favor **[abra uma Issue](https://github.com/alrigroup/ARDiscordBypass/issues)** detalhando:
- Seu sistema operacional
- Método de instalação do Discord (Flatpak, Snap, nativo, etc.)
- Mensagem de erro (se houver)
- Passos para reproduzir o problema

---

## 🤝 Contribuições

**Contribuições são 100% bem-vindas!** Se você quer melhorar o projeto, sinta-se à vontade para:

1. Fork do repositório
2. Criar uma branch para sua feature (`git checkout -b feature/nova-feature`)
3. Commit suas mudanças (`git commit -m 'Adiciona nova feature'`)
4. Push para a branch (`git push origin feature/nova-feature`)
5. Abrir um Pull Request

---

## 📄 Licença

Este projeto é protegido sob os termos da licença **ARGLFU (ALRI GROUP LICENSE FREE USE) Version 2 – 2026**.

- 🟢 **Uso e Distribuição**: Gratuito para uso pessoal e não comercial em sua forma original.
- 🔴 **Proibição de Modificação**: Não é permitida a alteração, modificação ou criação de obras derivadas do código sem autorização expressa do ALRI Group.
- 🔴 **Restrição Comercial**: Proibida qualquer venda, aluguel ou monetização deste software.

Para mais detalhes, consulte o arquivo [LICENSE](LICENSE) ou acesse os links oficiais:
- 🔗 **Repositório de Licenças**: [https://github.com/alrigroup/licenses](https://github.com/alrigroup/licenses)
- 📄 **Texto da Licença (RAW)**: [LICENSE-ARGLFU](https://raw.githubusercontent.com/alrigroup/licenses/refs/heads/main/LICENSE-ARGLFU)

---

## 👤 Créditos

- **Desenvolvimento & Autor**: Alexsanderalri.
- **Copyright**: Copyright © 2020-2026 **ALRI Group**. Todos os direitos reservados.
- **Linguagem**: C++20 / Win32 API / POSIX.
- **Contribuidores**: [Veja todos os contribuidores](https://github.com/alrigroup/ARDiscordBypass/graphs/contributors)

---

<p align="center">
  <sub><i>Aviso: Este projeto não é afiliado nem endossado pelo Discord Inc. Trata-se de uma ferramenta utilitária que faz uso de parâmetros nativos do executável Chromium.</i></sub>
</p>
