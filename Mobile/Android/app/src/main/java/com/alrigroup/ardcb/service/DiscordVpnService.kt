package com.alrigroup.ardcb.service

import android.app.Notification
import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.PendingIntent
import android.content.Intent
import android.os.Build
import androidx.core.app.NotificationCompat
import com.alrigroup.ardcb.MainActivity
import com.alrigroup.ardcb.tunnel.DiscordTunnel
import com.alrigroup.ardcb.tunnel.WireGuardConfigManager
import com.wireguard.android.backend.GoBackend
import com.wireguard.android.backend.Tunnel

class DiscordVpnService : GoBackend.VpnService() {

    companion object {
        const val CHANNEL_ID = "ardcb_vpn_channel"
        const val NOTIF_ID = 1001
        const val ACTION_STOP = "com.alrigroup.ardcb.STOP_VPN"

        val tunnel = DiscordTunnel("ARDiscordBypass")
        var isConnected = false
            private set

        private var stateListener: ((Boolean) -> Unit)? = null

        fun setStateListener(listener: (Boolean) -> Unit) {
            stateListener = listener
            listener(isConnected)
        }
    }

    private var backend: GoBackend? = null

    override fun onCreate() {
        super.onCreate()
        createNotificationChannel()
        backend = GoBackend(this)
        tunnel.setOnStateChangeListener { state ->
            val active = (state == Tunnel.State.UP)
            isConnected = active
            stateListener?.invoke(active)
            if (active) {
                updateNotification("Bypass Ativo: Transmissão e Câmera Liberados")
            }
        }
    }

    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        if (intent?.action == ACTION_STOP) {
            stopVpn()
            return START_NOT_STICKY
        }

        startForeground(NOTIF_ID, buildNotification("Iniciando túnel WireGuard isolado..."))

        Thread {
            try {
                val config = WireGuardConfigManager.createConfig(this)
                backend?.setState(tunnel, Tunnel.State.UP, config)
            } catch (e: Exception) {
                stopVpn()
            }
        }.start()

        return START_STICKY
    }

    private fun stopVpn() {
        Thread {
            try {
                backend?.setState(tunnel, Tunnel.State.DOWN, null)
            } catch (_: Exception) {}
            isConnected = false
            stateListener?.invoke(false)
            stopForeground(STOP_FOREGROUND_REMOVE)
            stopSelf()
        }.start()
    }

    override fun onDestroy() {
        stopVpn()
        super.onDestroy()
    }

    private fun createNotificationChannel() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            val channel = NotificationChannel(
                CHANNEL_ID,
                "ARDiscordBypass Status",
                NotificationManager.IMPORTANCE_LOW
            ).apply {
                description = "Notificação de serviço contínuo do bypass do Discord"
            }
            val manager = getSystemService(NotificationManager::class.java)
            manager.createNotificationChannel(channel)
        }
    }

    private fun buildNotification(text: String): Notification {
        val openIntent = Intent(this, MainActivity::class.java)
        val pOpen = PendingIntent.getActivity(
            this, 0, openIntent,
            PendingIntent.FLAG_IMMUTABLE or PendingIntent.FLAG_UPDATE_CURRENT
        )

        val stopIntent = Intent(this, DiscordVpnService::class.java).apply {
            action = ACTION_STOP
        }
        val pStop = PendingIntent.getService(
            this, 1, stopIntent,
            PendingIntent.FLAG_IMMUTABLE or PendingIntent.FLAG_UPDATE_CURRENT
        )

        return NotificationCompat.Builder(this, CHANNEL_ID)
            .setContentTitle("ARDiscordBypass v3.0 (ALRI GROUP)")
            .setContentText(text)
            .setSmallIcon(android.R.drawable.ic_lock_lock)
            .setContentIntent(pOpen)
            .addAction(android.R.drawable.ic_menu_close_clear_cancel, "Desconectar", pStop)
            .setOngoing(true)
            .build()
    }

    private fun updateNotification(text: String) {
        val manager = getSystemService(NotificationManager::class.java)
        manager.notify(NOTIF_ID, buildNotification(text))
    }
}
