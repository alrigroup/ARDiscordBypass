package com.alrigroup.ardcb.tile

import android.content.Intent
import android.net.VpnService
import android.os.Build
import android.service.quicksettings.Tile
import android.service.quicksettings.TileService
import androidx.annotation.RequiresApi
import androidx.core.content.ContextCompat
import com.alrigroup.ardcb.service.DiscordVpnService

@RequiresApi(Build.VERSION_CODES.N)
class BypassQuickTile : TileService() {

    override fun onStartListening() {
        super.onStartListening()
        updateTileState(DiscordVpnService.isConnected)
    }

    override fun onClick() {
        super.onClick()
        if (DiscordVpnService.isConnected) {
            val intent = Intent(this, DiscordVpnService::class.java).apply {
                action = DiscordVpnService.ACTION_STOP
            }
            startService(intent)
            updateTileState(false)
        } else {
            val vpnIntent = VpnService.prepare(this)
            if (vpnIntent == null) {
                val intent = Intent(this, DiscordVpnService::class.java)
                ContextCompat.startForegroundService(this, intent)
                updateTileState(true)
            } else {
                // Se precisa de permissão, abre a MainActivity
                vpnIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
                startActivityAndCollapse(vpnIntent)
            }
        }
    }

    private fun updateTileState(active: Boolean) {
        val tile = qsTile ?: return
        tile.state = if (active) Tile.STATE_ACTIVE else Tile.STATE_INACTIVE
        tile.label = if (active) "Bypass Ativo" else "ARDiscordBypass"
        tile.updateTile()
    }
}
