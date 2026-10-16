# 📱 ARDiscordBypass — Android Mobile Engine

Documentação de arquitetura, compilação e diretrizes de transparência de código aberto do cliente Android do **ARDiscordBypass v3.5.0**.

---

## 📑 Sumário

1. [Visão Geral & Arquitetura](#1-visão-geral--arquitetura)
2. [Transparência Open-Source & Biblioteca Nativa (`libprotonconfgen.so`)](#2-transparência-open-source--biblioteca-nativa-libprotonconfgenso)
   - [Por que o arquivo é `.so` e fica em `jniLibs`?](#por-que-o-arquivo-é-so-e-fica-em-jnilibs)
   - [Onde está o código-fonte?](#onde-está-o-código-fonte)
   - [Fallback de DNS para Android em Go Puro](#fallback-de-dns-para-android-em-go-puro)
3. [Split-Tunneling Estrito por UID de Aplicativo](#3-split-tunneling-estrito-por-uid-de-aplicativo)
4. [Instruções de Compilação & Build](#4-instruções-de-compilação--build)
   - [Passo 1: Recompilar o motor Go (Opcional)](#passo-1-recompilar-o-motor-go-opcional)
   - [Passo 2: Compilar o APK com Gradle](#passo-2-compilar-o-apk-com-gradle)
   - [Passo 3: Instalação via ADB](#passo-3-instalação-via-adb)
5. [Segurança e Diretrizes Antiban](#5-segurança-e-diretrizes-antiban)

---

## 1. Visão Geral & Arquitetura

O aplicativo Android do **ARDiscordBypass** foi desenvolvido em **Kotlin nativo** utilizando o componente `android.net.VpnService` em conjunto com a biblioteca oficial `com.wireguard.android:tunnel`.

O aplicativo é estruturado em:
* **Interface Monospace Cyberpunk / OLED Black**: 4 abas organizadas em categorias (Dashboard, Configurações, Logs, Sobre) com transição de opacidade desacelerada e animações de status em tempo real.
* **Console de Logs Retro-Moderno**: Renderiza a saída padrão do túnel WireGuard e do motor de autenticação com timestamps `[HH:mm:ss]`, coloração sintática e prompt animado piscante (`alri@ardcb:~$ █`).
* **Autenticação In-App Proton**: Geração de chaves e handshake SRP diretamente no dispositivo móvel, sem necessidade de computador ou ferramentas externas.
* **Modal de Limpeza Total**: Permite revogar túneis, apagar sessões salvas e restaurar o estado de fábrica com 1 toque.

---

## 2. Transparência Open-Source & Biblioteca Nativa (`libprotonconfgen.so`)

> ⚠️ **DECLARAÇÃO DE TRANSPARÊNCIA: 100% Código Aberto, Zero Blobs Binários Proprietários.**

### Onde está o código-fonte?
O arquivo localizado em:
```
Mobile/Android/app/src/main/jniLibs/arm64-v8a/libprotonconfgen.so
```
**NÃO É** uma biblioteca de código fechado. Seu código-fonte reside integralmente neste mesmo repositório na pasta:
```
tools/proton-confgen/
```
Ele é escrito em **Go 1.22+** e implementa o protocolo criptográfico oficial SRP (*Secure Remote Password* - RFC 5054) da ProtonVPN, uTLS client fingerprinting e geração de chaves Curve25519 para o WireGuard.

### Por que o arquivo é `.so` e fica em `jniLibs`?
No Android moderno (Android 10 / API 29 até o Android 15), o kernel do sistema operacional aplica restrições estritas de segurança:
1. **Políticas de W^X (*Write XOR Execute*) e SELinux**:
   Qualquer aplicativo que tente extrair um binário executável em seus diretórios de dados internos (`/data/data/com.alrigroup.ardcb/` ou `/data/user/0/...`) e executá-lo via `ProcessBuilder` ou `Runtime.getRuntime().exec()` é **terminado pelo kernel com erro de permissão**:
   ```
   java.io.IOException: Cannot run program: error=13, Permission denied (EACCES)
   ```
   Mesmo com chamadas `chmod +x`, a partição interna do app é montada com a flag `noexec`.
2. **Padrão Oficial do Android**:
   Para empacotar código executável de apoio sem exigir privilégios de Root do usuário, o sistema operacional Android exige que o arquivo seja nomeado com a convenção de biblioteca compartilhada (`lib<nome>.so`) e alocado no diretório de arquitetura correspondente (`jniLibs/arm64-v8a/`).
3. **Instalação pelo Package Manager**:
   Com a flag `useLegacyPackaging = true` no `build.gradle.kts`, o instalador do Android extrai o arquivo para a partição protegida de bibliotecas do sistema:
   ```
   /data/app/.../lib/arm64/libprotonconfgen.so
   ```
   Essa pasta é montada com permissões de execução legítimas (`PROT_EXEC` / `+x`), permitindo que a classe `ProtonAuthManager.kt` execute o processo com segurança e capture a saída stdout linha por linha para animar o console.

### Fallback de DNS para Android em Go Puro
Ao compilar programas Go puro sem CGO para Android (`CGO_ENABLED=0`), o runtime do Go tenta consultar servidores DNS lendo `/etc/resolv.conf`, arquivo que não existe no Android (que gerencia DNS via daemon IPC `netd`). Isso causava falhas do tipo `lookup api.protonvpn.ch: no such host`.

Para sanar isso de forma 100% nativa e portável, o arquivo `tools/proton-confgen/cmd/protonvpn-wg/main.go` possui um resolver padrão explicitamente injetado:
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

---

## 3. Split-Tunneling Estrito por UID de Aplicativo

O ARDiscordBypass Mobile utiliza `addAllowedApplication` da API `VpnService.Builder`. Apenas os pacotes correspondentes aos aplicativos do Discord têm seu tráfego roteado para a interface virtual `tun0`:

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

**Garantia para o Usuário:**
* Jogos mobile (Free Fire, COD, Genshin Impact, etc.) continuam 100% na conexão normal com ping inalterado.
* Aplicativos bancários e mensageiros (WhatsApp, Telegram) não passam pelo túnel e não sofrem alertas de segurança por mudança de IP.

---

## 4. Instruções de Compilação & Build

### Passo 1: Recompilar o motor Go (Opcional)
Se você realizou alterações em `tools/proton-confgen/`, recompile o binário arm64:
```bash
cd tools/proton-confgen
CGO_ENABLED=0 GOOS=android GOARCH=arm64 go build -ldflags "-s -w" -o libprotonconfgen.so cmd/protonvpn-wg/main.go
cp libprotonconfgen.so ../../Mobile/Android/app/src/main/jniLibs/arm64-v8a/libprotonconfgen.so
cd ../../Mobile/Android
```

### Passo 2: Compilar o APK com Gradle
Requisitos: JDK 17 e Android SDK instalado.
```bash
cd Mobile/Android
./gradlew assembleDebug
```
O arquivo `.apk` final será gerado em:
```
Mobile/Android/app/build/outputs/apk/debug/app-debug.apk
```

### Passo 3: Instalação via ADB
Com o smartphone conectado via cabo USB e depuração USB ativada:
```bash
adb install -r app/build/outputs/apk/debug/app-debug.apk
```

---

## 5. Segurança e Diretrizes Antiban

1. **Zero Modificação**: O APK do Discord não é modificado nem recompilado.
2. **Zero Injeção**: O aplicativo utiliza apenas as APIs padrão de VPN do sistema operacional Android.
3. **Chaves Privadas Seguras**: O par de chaves WireGuard é gerado localmente na memória RAM do aparelho e gravado exclusivamente no armazenamento interno privado do app (`ardc_vpn.conf`).
4. **Sem Telemetria**: O ARDiscordBypass não envia relatórios, dados de uso ou senhas para nenhum servidor terceiro ou próprio da ALRI GROUP. A comunicação ocorre exclusivamente entre o aparelho do usuário e os endpoints da ProtonVPN / Discord.
