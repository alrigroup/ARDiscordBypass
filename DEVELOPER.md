# ARDiscordBypass - Developer Docs

Documentação técnica avançada para desenvolvedores que querem entender o funcionamento interno do programa.

---

## Arquitetura

```
ARDiscordBypass/
├── main.cpp              ← Entry point + lógica de fluxo (cross-platform)
├── os/
│   ├── linux/
│   │   └── platform.h    ← Todas as funções específicas do Linux
│   └── windows/
│       └── platform.h    ← Todas as funções específicas do Windows
├── CMakeLists.txt
├── build.sh
└── build.bat
```

A separação por OS permite que `main.cpp` contenha apenas lógica de negócio, enquanto cada `platform.h` implementa as funções nativas de cada sistema.

---

## Fluxo de Execução

### 1. Inicialização
```
printBanner() → checkForUpdate() → locateDiscordExe()
```

### 2. Verificação de Update
- Consulta a API do GitHub Releases: `https://api.github.com/repos/alrigroup/ARDiscordBypass/releases/latest`
- Extrai o `tag_name` via regex
- Compara com `APP_VERSION` definido em `main.cpp`
- **Windows**: Usa `WinHTTP` (lib nativa)
- **Linux**: Usa `curl` via `runCommand()`

### 3. Detecção do Discord

**Ordem de busca no Linux:**
1. `which discord` / `command -v discord`
2. Flatpak (`/var/lib/flatpak/app/com.discordapp.Discord/...`)
3. Snap (`/snap/discord/current/`)
4. XDG (`~/.config/discord/`, `~/.local/share/discord/`)
5. Sistema (`/opt/Discord`, `/usr/share/discord`, `/usr/lib/discord`)
6. Argumento manual (se fornecido)

**Ordem de busca no Windows:**
1. `%LOCALAPPDATA%\Discord`
2. `%ProgramFiles%\Discord`
3. `%ProgramFiles(x86)%\Discord`
4. Argumento manual (se fornecido)

### 4. Busca de Proxy Automática

O programa consulta a API pública do ProxyScrape:
```
https://api.proxyscrape.com/v4/free-proxy-list/get?request=display_proxies&proxy_format=protocolipport&format=text&protocol=socks5&timeout=5000
```

- Baixa até 258 proxies SOCKS5
- Testa cada um contra `https://discord.com/api/v10/gateway`
- Usa o primeiro que retornar uma resposta válida
- Testa no máximo 15 proxies para não demorar muito

### 5. Lançamento do Discord

**Windows (`os/windows/platform.h`):**
- Usa `CreateProcessW()` com flags `CREATE_NO_WINDOW | DETACHED_PROCESS`
- Passa `--proxy-server` e `--proxy-bypass-list` como argumentos de linha de comando
- Redireciona stdout/stderr para `NUL`

**Linux - Nativo (`os/linux/platform.h`):**
- Usa `fork()` + `execl()`
- Passa `--proxy-server` e `--proxy-bypass-list` diretamente
- Redireciona stdout/stderr para `/dev/null`

**Linux - Flatpak:**
- Cria o arquivo `~/.var/app/com.discordapp.Discord/config/discord-flags.conf`
- O script interno do Flatpak (`/app/bin/com.discordapp.Discord`) lê esse arquivo automaticamente
- O conteúdo do arquivo é passado como flags para o Discord
- O arquivo é removido após 3 segundos

**Linux - Snap:**
- Cria o arquivo `~/snap/discord/common/discord-flags.conf`
- Mesma lógica do Flatpak

---

## Como o `discord-flags.conf` Funciona

O Flatpak do Discord contém um script wrapper em `/app/bin/com.discordapp.Discord` que:

1. Verifica se `${XDG_CONFIG_HOME}/discord-flags.conf` existe
2. Lê as linhas do arquivo (ignora linhas vazias e comentários `#`)
3. Passa cada linha como argumento para o executável do Discord

Exemplo de conteúdo:
```
--proxy-server=socks5://132.243.120.54:808
--proxy-bypass-list=cdn.discordapp.com;*.discordapp.net;*.discord.media;<local>
```

---

## Proxy Bypass List

A bypass list garante que apenas a verificação geográfica passe pelo proxy. Todo o resto (mídia, voz, vídeo) vai direto:

| Domínio | Tipo | Ação |
|---------|------|------|
| `cdn.discordapp.com` | CDN de imagens/assets | Bypass (direto) |
| `*.discordapp.net` | Servidores de mídia | Bypass (direto) |
| `*.discord.media` | WebRTC/Voice | Bypass (direto) |
| `<local>` | Conexões locais | Bypass (direto) |

---

## Compilação Avançada

### Dependências
- **Linux**: `libstdc++` (GCC) ou `libc++` (Clang) - já vem no sistema
- **Windows**: `ws2_32.lib`, `winhttp.lib`, `shell32.lib` - já vem no SDK

### Flags de Compilação Recomendadas

**Release (máxima otimização):**
```bash
# Linux
g++ -O3 -std=c++20 -march=x86-64 -DNDEBUG main.cpp -o ARDiscordBypass

# Windows (MSVC)
cl /O2 /std:c++20 /EHsc /DNDEBUG main.cpp ws2_32.lib winhttp.lib shell32.lib

# Windows (MinGW - linkagem estática)
g++ -O2 -std=c++20 -static -DNDEBUG main.cpp -lws2_32 -lwinhttp -lshell32 -o ARDiscordBypass.exe
```

> **Importante no Windows**: O flag `-static` é essencial. Sem ele, o executável depende de DLLs do sistema que podem não estar presentes em outras máquinas. Com `-static`, tudo é embutido no `.exe`.

**Debug (para desenvolvimento):**
```bash
# Linux
g++ -g -O0 -std=c++20 -fsanitize=address main.cpp -o ARDiscordBypass_debug

# Windows
cl /Od /std:c++20 /EHsc /Zi main.cpp ws2_32.lib winhttp.lib shell32.lib
```

### Cross-Compilation (Linux → Windows)
```bash
x86_64-w64-mingw32-g++ -O2 -std=c++20 main.cpp -lws2_32 -lwinhttp -lshell32 -o ARDiscordBypass.exe
```

---

## API do ProxyScrape

Endpoints utilizados:

| Endpoint | Descrição |
|----------|-----------|
| `GET /v4/free-proxy-list/get` | Lista de proxies gratuitos |
| `?request=display_proxies` | Formato simples (ip:port) |
| `&proxy_format=protocolipport` | Inclui protocolo (socks5://) |
| `&format=text` | Resposta em texto puro |
| `&protocol=socks5` | Apenas proxies SOCKS5 |
| `&timeout=5000` | Timeout de 5 segundos |

Rate limit: sem limite documentado, mas recomenda-se no máximo 1 request por execução.

---

## Estrutura de Headers (Linux)

Todas as funções do Linux estão em `os/linux/platform.h` como `inline`:

| Função | Descrição |
|--------|-----------|
| `runCommand()` | Executa comando via `popen()` e retorna stdout |
| `isDiscordRunning()` | Verifica via `pgrep -x Discord` |
| `killDiscord()` | Mata processo via `pkill -TERM` e `pkill -KILL` |
| `findDiscordExeIn()` | Busca Discord em um diretório (versão `app-*`) |
| `findDiscordExeFlatpak()` | Busca em Flatpak (system + user) |
| `findDiscordExeSnap()` | Busca em Snap |
| `findDiscordExeSystem()` | Busca em `/opt`, `/usr/bin`, etc. |
| `findDiscordExeXdg()` | Busca via XDG dirs |
| `findDiscordExeWhich()` | Busca via `which` |
| `locateDiscordExe()` | Orquestra todas as buscas |
| `isFlatpakPath()` | Detecta se caminho é Flatpak |
| `isSnapPath()` | Detecta se caminho é Snap |
| `getDiscordFlagsPath()` | Retorna path do `discord-flags.conf` |
| `writeDiscordFlags()` | Cria o arquivo de flags |
| `removeDiscordFlags()` | Remove o arquivo de flags |
| `launchDiscord()` | Fork + exec com proxy |
| `fetchProxies()` | Busca proxies da API |
| `testProxy()` | Testa se proxy funciona |
| `findWorkingProxy()` | Orquestra busca e teste |

---

## Possíveis Melhorias Futuras

- [ ] Proxy caching (evitar buscar da API toda execução)
- [ ] Suporte a proxy HTTP/HTTPS além de SOCKS5
- [ ] Verificação de proxy por região (país de saída)
- [ ] Modo headless (sem interface, apenas bypass)
- [ ] Suporte a múltiplos proxies com fallback
- [ ] Integração com systemd (Linux) para auto-start
- [ ] Suporte a ARM64 (Apple Silicon, Raspberry Pi)
