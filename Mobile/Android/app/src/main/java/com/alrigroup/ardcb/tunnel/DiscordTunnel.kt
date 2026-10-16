package com.alrigroup.ardcb.tunnel

import com.wireguard.android.backend.Tunnel

class DiscordTunnel(private val name: String = "ARDiscordBypass") : Tunnel {
    private var state: Tunnel.State = Tunnel.State.DOWN
    private var listener: ((Tunnel.State) -> Unit)? = null

    fun setOnStateChangeListener(listener: (Tunnel.State) -> Unit) {
        this.listener = listener
    }

    override fun getName(): String = name

    override fun onStateChange(newState: Tunnel.State) {
        state = newState
        listener?.invoke(newState)
    }

    fun getState(): Tunnel.State = state
}
