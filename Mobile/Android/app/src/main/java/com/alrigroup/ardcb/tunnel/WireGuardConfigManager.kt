package com.alrigroup.ardcb.tunnel

import android.content.Context
import android.content.pm.PackageManager
import com.wireguard.config.Config
import com.wireguard.config.Interface
import com.wireguard.config.Peer
import com.wireguard.config.InetNetwork
import com.wireguard.config.InetEndpoint
import java.net.InetAddress

object WireGuardConfigManager {
    private const val DNS_SERVER = "1.1.1.1"

    private val TARGET_PACKAGES = listOf(
        "com.discord",
        "com.discord.canary",
        "com.discord.ptb",
        "com.aliucord",
        "com.vendicated.vencord"
    )

    fun createConfig(context: Context): Config {
        val pm = context.packageManager
        val installedTargets = mutableListOf<String>()
        for (pkg in TARGET_PACKAGES) {
            try {
                pm.getPackageInfo(pkg, 0)
                installedTargets.add(pkg)
            } catch (_: PackageManager.NameNotFoundException) {}
        }
        val appsToInclude = if (installedTargets.isNotEmpty()) installedTargets else TARGET_PACKAGES

        // 1. Carregar ardc_vpn.conf (perfil WireGuard exclusivo e seguro do usuário)
        val customConf = java.io.File(context.filesDir, "ardc_vpn.conf")
        if (customConf.exists()) {
            val parsed = Config.parse(customConf.inputStream())
            val interfaceBuilder = Interface.Builder()
            interfaceBuilder.parsePrivateKey(parsed.`interface`.keyPair.privateKey.toBase64())
            for (addr in parsed.`interface`.addresses) interfaceBuilder.addAddress(addr)
            for (dns in parsed.`interface`.dnsServers) interfaceBuilder.addDnsServer(dns)
            interfaceBuilder.includeApplications(appsToInclude)

            val configBuilder = Config.Builder().setInterface(interfaceBuilder.build())
            for (peer in parsed.peers) configBuilder.addPeer(peer)
            return configBuilder.build()
        }

        throw IllegalStateException("Nenhum arquivo ardc_vpn.conf encontrado. Por favor, importe sua configuração WireGuard.")
    }

    fun hasConfig(context: Context): Boolean {
        return java.io.File(context.filesDir, "ardc_vpn.conf").exists()
    }

    fun deleteConfig(context: Context): Boolean {
        return try {
            val f = java.io.File(context.filesDir, "ardc_vpn.conf")
            val s = java.io.File(context.filesDir, ".protonvpn-session.json")
            if (s.exists()) s.delete()
            if (f.exists()) f.delete() else true
        } catch (_: Exception) {
            false
        }
    }

    fun saveConfig(context: Context, inputStream: java.io.InputStream): Boolean {
        return try {
            val dest = java.io.File(context.filesDir, "ardc_vpn.conf")
            dest.outputStream().use { out ->
                inputStream.copyTo(out)
            }
            true
        } catch (_: Exception) {
            false
        }
    }

    fun isDiscordInstalled(context: Context): Boolean {
        val pm = context.packageManager
        for (pkg in TARGET_PACKAGES) {
            try {
                pm.getPackageInfo(pkg, 0)
                return true
            } catch (_: PackageManager.NameNotFoundException) {}
        }
        return false
    }

    fun getInstalledDiscordPackage(context: Context): String? {
        val pm = context.packageManager
        for (pkg in TARGET_PACKAGES) {
            try {
                pm.getPackageInfo(pkg, 0)
                return pkg
            } catch (_: PackageManager.NameNotFoundException) {}
        }
        return null
    }
}
