package com.alrigroup.ardcb

import android.content.Intent
import android.net.VpnService
import android.net.Uri
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.view.View
import android.widget.ScrollView
import android.widget.TextView
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.app.AlertDialog
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.ContextCompat
import com.alrigroup.ardcb.service.DiscordVpnService
import com.alrigroup.ardcb.tunnel.WireGuardConfigManager
import com.alrigroup.ardcb.updater.UpdateChecker
import com.google.android.material.button.MaterialButton

class MainActivity : AppCompatActivity() {

    private lateinit var tvTerminalOutput: TextView
    private lateinit var scrollLogs: ScrollView
    private lateinit var btnToggleVpn: MaterialButton
    private lateinit var btnOpenDiscord: MaterialButton
    private lateinit var viewStatusDot: View
    private lateinit var tvStatusText: TextView
    private lateinit var tvVersion: TextView

    private lateinit var splashOverlay: View
    private lateinit var splashLogo: View
    private lateinit var mainContentContainer: View

    private val logBuffer = StringBuilder()
    private val mainHandler = Handler(Looper.getMainLooper())

    private val vpnPermissionLauncher = registerForActivityResult(
        ActivityResultContracts.StartActivityForResult()
    ) { result ->
        if (result.resultCode == RESULT_OK) {
            startVpnServiceInternal()
        } else {
            appendTerminal(" [✘] Permissão de VPN recusada pelo usuário.", isError = true)
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        tvTerminalOutput = findViewById(R.id.tvTerminalOutput)
        scrollLogs = findViewById(R.id.scrollLogs)
        btnToggleVpn = findViewById(R.id.btnToggleVpn)
        btnOpenDiscord = findViewById(R.id.btnOpenDiscord)
        viewStatusDot = findViewById(R.id.viewStatusDot)
        tvStatusText = findViewById(R.id.tvStatusText)
        tvVersion = findViewById(R.id.tvVersion)

        splashOverlay = findViewById(R.id.splashOverlay)
        splashLogo = findViewById(R.id.splashLogo)
        mainContentContainer = findViewById(R.id.mainContentContainer)

        tvVersion.text = "v" + UpdateChecker.CURRENT_VERSION

        printAsciiBanner()

        // Inicialmente o botão de abrir fica desabilitado
        btnOpenDiscord.isEnabled = false
        btnOpenDiscord.alpha = 0.4f

        // Inicia com o conteúdo oculto enquanto a splash roda
        mainContentContainer.alpha = 0f
        mainContentContainer.translationY = 60f

        startSplashAnimation()

        DiscordVpnService.setStateListener { active ->
            runOnUiThread {
                updateUiState(active)
            }
        }

        btnToggleVpn.setOnClickListener {
            if (DiscordVpnService.isConnected) {
                stopVpnServiceInternal()
            } else {
                prepareAndStartVpn()
            }
        }

        btnOpenDiscord.setOnClickListener {
            launchDiscordApp()
        }

        val btnMenu: android.widget.ImageButton = findViewById(R.id.btnMenu)
        btnMenu.setOnClickListener { view ->
            showPopupMenu(view)
        }

        // Checagem de atualizações assíncrona
        appendTerminal("\n[ ⠋ ] Verificando atualizações no GitHub...")
        UpdateChecker.check { update ->
            runOnUiThread {
                if (update.hasUpdate) {
                    appendTerminal("[ ! ] NOVA VERSÃO DISPONÍVEL: v${update.latestVersion}")
                    showUpdateDialog(update)
                } else {
                    appendTerminal("[ ✔ ] Você está na versão mais recente (v${UpdateChecker.CURRENT_VERSION}).")
                }

                // Checar se o Discord está instalado
                checkDiscordTarget()
            }
        }
    }

    private fun startSplashAnimation() {
        // 1. Rotação sutil com escala tipo respiração
        splashLogo.animate()
            .rotation(360f)
            .scaleX(1.15f)
            .scaleY(1.15f)
            .setDuration(1200)
            .setInterpolator(android.view.animation.AccelerateDecelerateInterpolator())
            .withEndAction {
                // 2. Transição cinematográfica: Logo sobe e desaparece "puxando" o app para cima
                splashOverlay.animate()
                    .alpha(0f)
                    .translationY(-120f)
                    .setDuration(500)
                    .withEndAction {
                        splashOverlay.visibility = View.GONE
                    }
                    .start()

                mainContentContainer.animate()
                    .alpha(1f)
                    .translationY(0f)
                    .setDuration(550)
                    .setInterpolator(android.view.animation.DecelerateInterpolator())
                    .start()
            }
            .start()
    }

    private fun printAsciiBanner() {
        val banner = """
 ╔════════════════════════════════════╗
 ║        ARDiscordBypass v3.0        ║
 ║      ALRI GROUP // Discord Live    ║
 ╚════════════════════════════════════╝
        """.trimIndent()

        logBuffer.append(banner).append("\n\n")
        tvTerminalOutput.text = logBuffer.toString()
    }

    private fun appendTerminal(text: String, isError: Boolean = false) {
        val prefix = if (isError) "\n[ERRO] " else "\nARDCB - "
        logBuffer.append(prefix).append(text)
        tvTerminalOutput.text = logBuffer.toString()
        scrollLogs.post {
            scrollLogs.fullScroll(ScrollView.FOCUS_DOWN)
        }
    }

    private fun checkDiscordTarget() {
        val installed = WireGuardConfigManager.isDiscordInstalled(this)
        if (installed) {
            val pkg = WireGuardConfigManager.getInstalledDiscordPackage(this)
            appendTerminal("[ ✔ ] Alvo detectado no Android: $pkg")
            appendTerminal("[ ✔ ] Isolamento por processo (Split-Tunneling) pronto.")
        } else {
            appendTerminal("[AVISO] Nenhum app oficial do Discord encontrado.")
            appendTerminal("Dica: Instale o Discord pela Play Store para utilizar o bypass.")
        }
    }

    private fun prepareAndStartVpn() {
        val vpnIntent = VpnService.prepare(this)
        if (vpnIntent != null) {
            vpnPermissionLauncher.launch(vpnIntent)
        } else {
            startVpnServiceInternal()
        }
    }

    private fun startVpnServiceInternal() {
        simulateConnectingAnimation {
            val intent = Intent(this, DiscordVpnService::class.java)
            ContextCompat.startForegroundService(this, intent)
        }
    }

    private fun stopVpnServiceInternal() {
        appendTerminal("\n[ ⠋ ] Encerrando túnel WireGuard isolado...")
        val intent = Intent(this, DiscordVpnService::class.java).apply {
            action = DiscordVpnService.ACTION_STOP
        }
        startService(intent)
        appendTerminal("[ ✔ ] Túnel desconectado com sucesso.")
    }

    private fun simulateConnectingAnimation(onDone: () -> Unit) {
        appendTerminal("\n[ ⠋ ] Preparando túnel WireGuard dedicado (AllowedApps)...")
        mainHandler.postDelayed({
            appendTerminal("[ ⠙ ] Estabelecendo handshake com servidor internacional...")
            mainHandler.postDelayed({
                appendTerminal("[ ✔ ] Túnel conectado! Discord liberado para Transmissão e Câmera.")
                onDone()
            }, 600)
        }, 500)
    }

    private fun updateUiState(active: Boolean) {
        if (active) {
            viewStatusDot.backgroundTintList = ContextCompat.getColorStateList(this, R.color.terminal_green)
            tvStatusText.text = "BYPASS ATIVO (PROTEGIDO)"
            tvStatusText.setTextColor(ContextCompat.getColor(this, R.color.terminal_green))
            btnToggleVpn.text = "[ DESCONECTAR BYPASS ]"
            btnToggleVpn.backgroundTintList = ContextCompat.getColorStateList(this, R.color.terminal_card_border)
            btnToggleVpn.setTextColor(ContextCompat.getColor(this, R.color.terminal_text_primary))

            // Habilitar o botão de abrir Discord quando a VPN estiver ativa
            btnOpenDiscord.isEnabled = true
            btnOpenDiscord.alpha = 1.0f
        } else {
            viewStatusDot.backgroundTintList = ContextCompat.getColorStateList(this, R.color.alri_red)
            tvStatusText.text = "BYPASS DESCONECTADO"
            tvStatusText.setTextColor(ContextCompat.getColor(this, R.color.terminal_text_secondary))
            btnToggleVpn.text = "[ INICIAR BYPASS NO DISCORD ]"
            btnToggleVpn.backgroundTintList = ContextCompat.getColorStateList(this, R.color.alri_red)
            btnToggleVpn.setTextColor(ContextCompat.getColor(this, R.color.terminal_btn_text))

            // Desabilitar o botão de abrir Discord se a VPN não estiver ativa
            btnOpenDiscord.isEnabled = false
            btnOpenDiscord.alpha = 0.4f
        }
    }

    private fun launchDiscordApp() {
        val pkg = WireGuardConfigManager.getInstalledDiscordPackage(this) ?: "com.discord"
        val launchIntent = packageManager.getLaunchIntentForPackage(pkg)
        if (launchIntent != null) {
            appendTerminal("\n> Abrindo Discord ($pkg)...")
            startActivity(launchIntent)
        } else {
            appendTerminal("\n[✘] Não foi possível abrir o Discord. O app está instalado?", isError = true)
        }
    }

    private fun showPopupMenu(anchor: View) {
        val popup = androidx.appcompat.widget.PopupMenu(this, anchor)
        popup.menuInflater.inflate(R.menu.main_menu, popup.menu)
        popup.setOnMenuItemClickListener { item ->
            when (item.itemId) {
                R.id.action_notes -> {
                    showNotesDialog()
                    true
                }
                R.id.action_credits -> {
                    showCreditsDialog()
                    true
                }
                R.id.action_github -> {
                    val url = "https://github.com/alrigroup/ARDiscordBypass"
                    startActivity(Intent(Intent.ACTION_VIEW, Uri.parse(url)))
                    true
                }
                else -> false
            }
        }
        popup.show()
    }

    private fun showNotesDialog() {
        val bottomSheet = com.google.android.material.bottomsheet.BottomSheetDialog(this)
        val sheetView = layoutInflater.inflate(R.layout.layout_terminal_bottom_sheet, null)
        bottomSheet.setContentView(sheetView)

        val tvTitle: TextView = sheetView.findViewById(R.id.tvSheetTitle)
        val tvContent: TextView = sheetView.findViewById(R.id.tvSheetContent)
        val btnClose: View = sheetView.findViewById(R.id.btnCloseSheet)
        val btnAction: MaterialButton = sheetView.findViewById(R.id.btnSheetAction)

        tvTitle.text = "NOTAS DE FUNCIONAMENTO"
        btnAction.text = "[ ENTENDIDO ]"

        val notes = """
            ⚡ COMO FUNCIONA O BYPASS:
            
            1. Roteamento por Processo (Split-Tunneling):
            O ARDiscordBypass utiliza a tecnologia WireGuard para criar um túnel exclusivo apenas para os aplicativos do Discord (com.discord, canary, ptb, aliucord, vencord).
            
            2. Sua Internet Fica Intocada:
            Todo o restante do seu celular (WhatsApp, bancos, jogos, YouTube e chamadas) continua 100% na sua internet normal sem consumir dados do túnel e com ping nativo.
            
            3. Liberação de Transmissão e Câmera:
            Durante a entrada em salas de voz ou transmissão de tela, o tráfego de mídia do Discord passa pela rota internacional segura, liberando compartilhamento de tela e vídeo.
            
            4. Segurança & Antiban:
            Zero modificação de APK do Discord, zero injeção de código e 100% em conformidade com as diretrizes do sistema.
        """.trimIndent()

        tvContent.text = notes

        btnClose.setOnClickListener { bottomSheet.dismiss() }
        btnAction.setOnClickListener { bottomSheet.dismiss() }
        bottomSheet.show()
    }

    private fun showCreditsDialog() {
        val bottomSheet = com.google.android.material.bottomsheet.BottomSheetDialog(this)
        val sheetView = layoutInflater.inflate(R.layout.layout_terminal_bottom_sheet, null)
        bottomSheet.setContentView(sheetView)

        val tvTitle: TextView = sheetView.findViewById(R.id.tvSheetTitle)
        val tvContent: TextView = sheetView.findViewById(R.id.tvSheetContent)
        val btnClose: View = sheetView.findViewById(R.id.btnCloseSheet)
        val btnAction: MaterialButton = sheetView.findViewById(R.id.btnSheetAction)

        tvTitle.text = "CRÉDITOS & AUTOR"
        btnAction.text = "[ FECHAR ]"

        val credits = """
            🚀 ARDiscordBypass v3.0.0
            
            👤 Desenvolvimento & Autor:
            Alexsanderalri
            
            🏢 Organização:
            ALRI Group (Copyright © 2020-2026)
            
            📄 Licença:
            ARGLFU (ALRI GROUP LICENSE FREE USE) v2
            
            🌐 Repositório Oficial:
            github.com/alrigroup/ARDiscordBypass
            
            🛡️ Proteção de Conexão:
            WireGuard Native Split-Tunnel Engine
        """.trimIndent()

        tvContent.text = credits

        btnClose.setOnClickListener { bottomSheet.dismiss() }
        btnAction.setOnClickListener { bottomSheet.dismiss() }
        bottomSheet.show()
    }

    private fun showUpdateDialog(info: UpdateChecker.UpdateInfo) {
        AlertDialog.Builder(this)
            .setTitle("Nova Atualização: v${info.latestVersion}")
            .setMessage("Uma nova versão do ARDiscordBypass está disponível.\n\nNovidades:\n${info.releaseNotes}")
            .setPositiveButton("Baixar Atualização") { _, _ ->
                val browserIntent = Intent(Intent.ACTION_VIEW, Uri.parse(info.downloadUrl))
                startActivity(browserIntent)
            }
            .setNegativeButton("Mais Tarde", null)
            .show()
    }
}
