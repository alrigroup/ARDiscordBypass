package com.alrigroup.ardcb.tunnel

import android.content.Context
import android.content.pm.PackageManager
import com.wireguard.config.Config
import com.wireguard.config.Interface
import com.wireguard.config.Peer
import com.wireguard.config.InetNetwork
import com.wireguard.config.InetEndpoint
import com.wireguard.crypto.Key
import java.net.InetAddress

object WireGuardConfigManager {
    // Configuração oficial validada do ARDiscordBypass
    private const val CLIENT_PRIVATE_KEY_B64 = "REDACTED_PRIVATE_KEY="
    private const val PEER_PUBLIC_KEY_B64 = "REDACTED_PUBLIC_KEY="
    private const val ENDPOINT_HOST = "127.0.0.1"
    private const val ENDPOINT_PORT = 51820
    private const val CLIENT_ADDRESS = "10.2.0.2/32"
    private const val DNS_SERVER = "1.1.1.1"

    // Pacotes do Discord e mods no Android que devem passar pelo túnel
    private val TARGET_PACKAGES = listOf(
        "com.discord",
        "com.discord.canary",
        "com.discord.ptb",
        "com.aliucord",
        "com.vendicated.vencord"
    )

    fun createConfig(context: Context): Config {
        val interfaceBuilder = Interface.Builder()
        interfaceBuilder.parsePrivateKey(CLIENT_PRIVATE_KEY_B64)
        interfaceBuilder.addAddress(InetNetwork.parse(CLIENT_ADDRESS))
        interfaceBuilder.addDnsServer(InetAddress.getByName(DNS_SERVER))

        // Filtrar apenas pacotes instalados ou permitir os conhecidos
        val pm = context.packageManager
        val installedTargets = mutableListOf<String>()
        for (pkg in TARGET_PACKAGES) {
            try {
                pm.getPackageInfo(pkg, 0)
                installedTargets.add(pkg)
            } catch (_: PackageManager.NameNotFoundException) {
                // Não instalado
            }
        }

        // Se nenhum estiver instalado ainda, adicionamos a lista padrão para garantir
        val appsToInclude = if (installedTargets.isNotEmpty()) installedTargets else TARGET_PACKAGES
        interfaceBuilder.includeApplications(appsToInclude)

        val peerBuilder = Peer.Builder()
        peerBuilder.parsePublicKey(PEER_PUBLIC_KEY_B64)
        peerBuilder.setEndpoint(InetEndpoint.parse("$ENDPOINT_HOST:$ENDPOINT_PORT"))
        peerBuilder.addAllowedIp(InetNetwork.parse("0.0.0.0/0"))
        peerBuilder.addAllowedIp(InetNetwork.parse("::/0"))
        peerBuilder.setPersistentKeepalive(25)

        return Config.Builder()
            .setInterface(interfaceBuilder.build())
            .addPeer(peerBuilder.build())
            .build()
    }

    fun isDiscordInstalled(context: Context): Boolean {
        val pm = context.packageManager
        for (pkg in TARGET_PACKAGES) {
            try {
                pm.getPackageInfo(pkg, 0)
                return true
            } catch (_: PackageManager.NameNotFoundException) {
            }
        }
        return false
    }

    fun getInstalledDiscordPackage(context: Context): String? {
        val pm = context.packageManager
        for (pkg in TARGET_PACKAGES) {
            try {
                pm.getPackageInfo(pkg, 0)
                return pkg
            } catch (_: PackageManager.NameNotFoundException) {
            }
        }
        return null
    }
}
