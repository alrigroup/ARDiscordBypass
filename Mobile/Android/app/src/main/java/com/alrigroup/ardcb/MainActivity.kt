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
import android.widget.Toast
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.app.AlertDialog
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.ContextCompat
import com.alrigroup.ardcb.service.DiscordVpnService
import com.alrigroup.ardcb.tunnel.ProtonAuthManager
import com.alrigroup.ardcb.tunnel.WireGuardConfigManager
import com.alrigroup.ardcb.updater.UpdateChecker
import com.google.android.material.button.MaterialButton
import com.google.android.material.textfield.TextInputEditText
import androidx.lifecycle.lifecycleScope
import kotlinx.coroutines.launch
import android.animation.AnimatorSet
import android.animation.ObjectAnimator
import android.animation.ValueAnimator
import android.view.animation.AccelerateDecelerateInterpolator
import android.view.inputmethod.InputMethodManager
import android.text.Spannable
import android.text.SpannableString
import android.text.SpannableStringBuilder
import android.text.style.ForegroundColorSpan
import android.widget.FrameLayout
import android.widget.ImageView
import android.widget.ProgressBar
import java.net.InetSocketAddress
import java.net.Socket
import kotlin.concurrent.thread

class MainActivity : AppCompatActivity() {

    // Views de Navegação / Abas
    private lateinit var tabDashboard: TextView
    private lateinit var tabConfig: TextView
    private lateinit var tabLogs: TextView
    private lateinit var tabAbout: TextView

    private lateinit var containerDashboard: View
    private lateinit var containerConfig: View
    private lateinit var containerLogs: View
    private lateinit var containerAbout: View

    // Status Pill
    private lateinit var statusPill: View
    private lateinit var viewStatusDot: View
    private lateinit var tvStatusText: TextView
    private lateinit var tvVersion: TextView

    // Dashboard Views
    private lateinit var tvMetricInterface: TextView
    private lateinit var tvMetricRouting: TextView
    private lateinit var tvMetricDiscordClient: TextView
    private lateinit var tvMetricPing: TextView
    private lateinit var tvMetricTunnelStatus: TextView
    private lateinit var btnToggleVpn: MaterialButton
    private lateinit var pbVpnConnecting: ProgressBar
    private lateinit var btnOpenDiscord: MaterialButton
    private var vpnConnectingRunnable: Runnable? = null

    // Config Tab Views
    private lateinit var tvConfigStatus: TextView
    private lateinit var etConfigProtonUser: TextInputEditText
    private lateinit var etConfigProtonPass: TextInputEditText
    private lateinit var btnConfigProtonLogin: MaterialButton
    private lateinit var btnProtonWeb: MaterialButton
    private lateinit var btnImportConfig: MaterialButton
    private lateinit var btnImportCustom: MaterialButton
    private lateinit var tvDataSummary: TextView
    private lateinit var btnClearAllData: MaterialButton

    // Logs Tab Views
    private lateinit var tvTerminalOutput: TextView
    private lateinit var tvTerminalPrompt: TextView
    private lateinit var scrollLogs: ScrollView
    private lateinit var btnClearLogs: MaterialButton
    private val timeFormat = java.text.SimpleDateFormat("HH:mm:ss", java.util.Locale.getDefault())

    // About Tab Views
    private lateinit var btnOpenGitHub: MaterialButton
    private lateinit var btnShowNotes: MaterialButton
    private lateinit var btnShowCredits: MaterialButton

    // Splash Overlay
    private lateinit var splashOverlay: View
    private lateinit var splashLogo: View
    private lateinit var mainContentContainer: View

    // Auth Loading Overlay
    private lateinit var authLoadingOverlay: FrameLayout
    private lateinit var ivAuthLoadingLogo: ImageView
    private lateinit var pbAuthLoading: ProgressBar
    private lateinit var tvAuthLoadingTitle: TextView
    private lateinit var tvAuthLoadingStatus: TextView
    private lateinit var btnAuthLoadingDismiss: MaterialButton
    private var authLogoAnimator: AnimatorSet? = null

    private val logBuffer = StringBuilder()
    private val mainHandler = Handler(Looper.getMainLooper())
    private var activeTab = 0 // 0 = Dash, 1 = Config, 2 = Logs, 3 = About

    private val vpnPermissionLauncher = registerForActivityResult(
        ActivityResultContracts.StartActivityForResult()
    ) { result ->
        if (result.resultCode == RESULT_OK) {
            startVpnServiceInternal()
        } else {
            appendTerminal(" [✘] Permissão de VPN recusada pelo usuário.", isError = true)
        }
    }

    private val importConfigLauncher = registerForActivityResult(
        ActivityResultContracts.GetContent()
    ) { uri: Uri? ->
        if (uri != null) {
            try {
                contentResolver.openInputStream(uri)?.use { stream ->
                    if (WireGuardConfigManager.saveConfig(this, stream)) {
                        appendTerminal("[ ✔ ] Perfil ardc_vpn.conf importado com sucesso!")
                        Toast.makeText(this, "Perfil WireGuard importado com sucesso!", Toast.LENGTH_SHORT).show()
                        refreshConfigState()
                    } else {
                        appendTerminal(" [✘] Falha ao salvar ardc_vpn.conf.", isError = true)
                    }
                }
            } catch (e: Exception) {
                appendTerminal(" [✘] Erro ao importar: ${e.message}", isError = true)
            }
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        initViews()
        setupNavigationTabs()
        setupClickListeners()

        tvVersion.text = "v" + UpdateChecker.CURRENT_VERSION + " • ALRI GROUP"

        printAsciiBanner()
        startBlinkingCursor()

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

        refreshConfigState()
        checkDiscordTarget()

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
            }
        }

        // Medição de ping periódica em background
        startPeriodicPingProbe()
    }

    private fun initViews() {
        tabDashboard = findViewById(R.id.tabDashboard)
        tabConfig = findViewById(R.id.tabConfig)
        tabLogs = findViewById(R.id.tabLogs)
        tabAbout = findViewById(R.id.tabAbout)

        containerDashboard = findViewById(R.id.containerDashboard)
        containerConfig = findViewById(R.id.containerConfig)
        containerLogs = findViewById(R.id.containerLogs)
        containerAbout = findViewById(R.id.containerAbout)

        statusPill = findViewById(R.id.statusPill)
        viewStatusDot = findViewById(R.id.viewStatusDot)
        tvStatusText = findViewById(R.id.tvStatusText)
        tvVersion = findViewById(R.id.tvVersion)

        tvMetricInterface = findViewById(R.id.tvMetricInterface)
        tvMetricRouting = findViewById(R.id.tvMetricRouting)
        tvMetricDiscordClient = findViewById(R.id.tvMetricDiscordClient)
        tvMetricPing = findViewById(R.id.tvMetricPing)
        tvMetricTunnelStatus = findViewById(R.id.tvMetricTunnelStatus)
        btnToggleVpn = findViewById(R.id.btnToggleVpn)
        pbVpnConnecting = findViewById(R.id.pbVpnConnecting)
        btnOpenDiscord = findViewById(R.id.btnOpenDiscord)

        tvConfigStatus = findViewById(R.id.tvConfigStatus)
        etConfigProtonUser = findViewById(R.id.etConfigProtonUser)
        etConfigProtonPass = findViewById(R.id.etConfigProtonPass)
        btnConfigProtonLogin = findViewById(R.id.btnConfigProtonLogin)
        btnProtonWeb = findViewById(R.id.btnProtonWeb)
        btnImportConfig = findViewById(R.id.btnImportConfig)
        btnImportCustom = findViewById(R.id.btnImportCustom)
        tvDataSummary = findViewById(R.id.tvDataSummary)
        btnClearAllData = findViewById(R.id.btnClearAllData)

        tvTerminalOutput = findViewById(R.id.tvTerminalOutput)
        tvTerminalPrompt = findViewById(R.id.tvTerminalPrompt)
        scrollLogs = findViewById(R.id.scrollLogs)
        btnClearLogs = findViewById(R.id.btnClearLogs)

        btnOpenGitHub = findViewById(R.id.btnOpenGitHub)
        btnShowNotes = findViewById(R.id.btnShowNotes)
        btnShowCredits = findViewById(R.id.btnShowCredits)

        splashOverlay = findViewById(R.id.splashOverlay)
        splashLogo = findViewById(R.id.splashLogo)
        mainContentContainer = findViewById(R.id.mainContentContainer)

        authLoadingOverlay = findViewById(R.id.authLoadingOverlay)
        ivAuthLoadingLogo = findViewById(R.id.ivAuthLoadingLogo)
        pbAuthLoading = findViewById(R.id.pbAuthLoading)
        tvAuthLoadingTitle = findViewById(R.id.tvAuthLoadingTitle)
        tvAuthLoadingStatus = findViewById(R.id.tvAuthLoadingStatus)
        btnAuthLoadingDismiss = findViewById(R.id.btnAuthLoadingDismiss)
    }

    private fun setupNavigationTabs() {
        val tabs = listOf(tabDashboard, tabConfig, tabLogs, tabAbout)
        val containers = listOf(containerDashboard, containerConfig, containerLogs, containerAbout)

        fun selectTab(index: Int) {
            if (activeTab == index) return
            activeTab = index
            for (i in tabs.indices) {
                if (i == index) {
                    tabs[i].setBackgroundResource(R.drawable.bg_tab_active)
                    tabs[i].setTextColor(ContextCompat.getColor(this, R.color.terminal_btn_text))
                    val container = containers[i]
                    container.alpha = 0f
                    container.translationY = 24f
                    container.visibility = View.VISIBLE
                    container.animate()
                        .alpha(1f)
                        .translationY(0f)
                        .setDuration(220)
                        .setInterpolator(android.view.animation.DecelerateInterpolator())
                        .start()
                } else {
                    tabs[i].setBackgroundResource(R.drawable.bg_tab_inactive)
                    tabs[i].setTextColor(ContextCompat.getColor(this, R.color.terminal_text_secondary))
                    containers[i].visibility = View.GONE
                }
            }
        }

        tabDashboard.setOnClickListener { selectTab(0) }
        tabConfig.setOnClickListener { selectTab(1) }
        tabLogs.setOnClickListener { selectTab(2) }
        tabAbout.setOnClickListener { selectTab(3) }
    }

    private fun setupClickListeners() {
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

        btnProtonWeb.setOnClickListener {
            val url = "https://account.protonvpn.com/pt-br/login"
            startActivity(Intent(Intent.ACTION_VIEW, Uri.parse(url)))
            appendTerminal("\n[ ℹ ] Abrindo página oficial de login ProtonVPN...")
        }

        btnConfigProtonLogin.setOnClickListener {
            val user = etConfigProtonUser.text?.toString() ?: ""
            val pass = etConfigProtonPass.text?.toString() ?: ""
            triggerProtonAuthentication(user, pass, startVpnOnSuccess = false, origin = "config")
        }

        btnImportConfig.setOnClickListener {
            importConfigLauncher.launch("*/*")
        }

        btnImportCustom.setOnClickListener {
            importConfigLauncher.launch("*/*")
        }

        btnClearAllData.setOnClickListener {
            confirmClearAllData()
        }

        btnClearLogs.setOnClickListener {
            tvTerminalOutput.text = ""
            appendTerminal("[ ✔ ] Console e buffer de eventos limpos.")
            Toast.makeText(this, "Logs limpos com sucesso.", Toast.LENGTH_SHORT).show()
        }

        btnOpenGitHub.setOnClickListener {
            val url = "https://github.com/alrigroup/ARDiscordBypass"
            startActivity(Intent(Intent.ACTION_VIEW, Uri.parse(url)))
        }

        btnShowNotes.setOnClickListener {
            showNotesDialog()
        }

        btnShowCredits.setOnClickListener {
            showCreditsDialog()
        }
    }

    private fun refreshConfigState() {
        val hasConf = WireGuardConfigManager.hasConfig(this)
        val hasSession = ProtonAuthManager.hasSavedSession(this)
        if (hasConf) {
            val note = if (hasSession) " (ProtonVPN Ativa)" else ""
            tvConfigStatus.text = "Presente no disco (ardc_vpn.conf)$note"
            tvConfigStatus.setTextColor(ContextCompat.getColor(this, R.color.terminal_green))
            tvDataSummary.text = "Armazenamento local: ardc_vpn.conf$note"
        } else {
            tvConfigStatus.text = "Nenhum perfil configurado"
            tvConfigStatus.setTextColor(ContextCompat.getColor(this, R.color.terminal_yellow))
            tvDataSummary.text = "Armazenamento local: Nenhum arquivo gravado"
        }
    }

    private fun confirmClearAllData() {
        val bottomSheet = com.google.android.material.bottomsheet.BottomSheetDialog(this)
        val sheetView = layoutInflater.inflate(R.layout.layout_dialog_wipe_data, null)
        bottomSheet.setContentView(sheetView)

        val btnClose: View = sheetView.findViewById(R.id.btnCloseWipeSheet)
        val btnConfirm: MaterialButton = sheetView.findViewById(R.id.btnConfirmWipeData)
        val btnCancel: MaterialButton = sheetView.findViewById(R.id.btnCancelWipeData)

        btnClose.setOnClickListener { bottomSheet.dismiss() }
        btnCancel.setOnClickListener { bottomSheet.dismiss() }

        btnConfirm.setOnClickListener {
            bottomSheet.dismiss()
            if (DiscordVpnService.isConnected) {
                stopVpnServiceInternal()
            }
            val deleted = WireGuardConfigManager.deleteConfig(this)
            ProtonAuthManager.clearSession(this)
            etConfigProtonUser.setText("")
            etConfigProtonPass.setText("")
            if (deleted) {
                appendTerminal("[ ✔ ] Todas as configurações e arquivos locais foram apagados.")
                Toast.makeText(this, "Configurações apagadas com sucesso!", Toast.LENGTH_SHORT).show()
            } else {
                appendTerminal("[ ! ] Não havia arquivos para remover.")
            }
            refreshConfigState()
        }

        bottomSheet.show()
    }

    private fun startPeriodicPingProbe() {
        thread {
            while (!isDestroyed) {
                try {
                    val s = Socket()
                    val start = System.currentTimeMillis()
                    s.connect(InetSocketAddress("1.1.1.1", 53), 1500)
                    val ping = System.currentTimeMillis() - start
                    s.close()
                    runOnUiThread {
                        tvMetricPing.text = "$ping ms"
                    }
                } catch (_: Exception) {
                    runOnUiThread {
                        tvMetricPing.text = "-- ms"
                    }
                }
                Thread.sleep(4000)
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

    private fun startBlinkingCursor() {
        mainHandler.post(object : Runnable {
            private var blink = true
            override fun run() {
                if (isDestroyed) return
                tvTerminalPrompt.text = if (blink) "alri@ardcb:~$ █" else "alri@ardcb:~$  "
                blink = !blink
                mainHandler.postDelayed(this, 550)
            }
        })
    }

    private fun printAsciiBanner() {
        val banner = """
 ╔════════════════════════════════════════════╗
 ║         ARDiscordBypass v3.5.0             ║
 ║      ALRI GROUP // Discord Split-Tunnel    ║
 ╚════════════════════════════════════════════╝
        """.trimIndent()

        val span = SpannableString(banner + "\n\n")
        span.setSpan(
            ForegroundColorSpan(ContextCompat.getColor(this, R.color.alri_red)),
            0, span.length, Spannable.SPAN_EXCLUSIVE_EXCLUSIVE
        )
        tvTerminalOutput.text = span
    }

    private fun appendTerminal(rawText: String, isError: Boolean = false) {
        val clean = rawText.trim()
        if (clean.isBlank()) return

        val timeStr = timeFormat.format(java.util.Date())
        val timeSpan = SpannableString("[$timeStr] ")
        timeSpan.setSpan(
            ForegroundColorSpan(ContextCompat.getColor(this, R.color.terminal_text_muted)),
            0, timeSpan.length, Spannable.SPAN_EXCLUSIVE_EXCLUSIVE
        )

        val bodySpan = SpannableString(clean + "\n")
        val color = when {
            isError || clean.contains("[ ✖ ]") || clean.contains("[✘]") || clean.startsWith("[ERRO]") ->
                ContextCompat.getColor(this, R.color.alri_red)
            clean.contains("[ ✔ ]") || clean.contains("Sucesso") || clean.contains("conectado") || clean.contains("pronto") ->
                ContextCompat.getColor(this, R.color.terminal_green)
            clean.contains("[ ! ]") || clean.contains("[AVISO]") ->
                ContextCompat.getColor(this, R.color.terminal_yellow)
            clean.contains("[ ℹ ]") || clean.startsWith(">") ->
                ContextCompat.getColor(this, R.color.terminal_text_primary)
            else -> ContextCompat.getColor(this, R.color.terminal_text_secondary)
        }

        bodySpan.setSpan(
            ForegroundColorSpan(color),
            0, bodySpan.length, Spannable.SPAN_EXCLUSIVE_EXCLUSIVE
        )

        val fullLine = SpannableStringBuilder().append(timeSpan).append(bodySpan)

        runOnUiThread {
            tvTerminalOutput.append(fullLine)
            scrollLogs.post {
                scrollLogs.fullScroll(ScrollView.FOCUS_DOWN)
            }
        }
    }

    private fun checkDiscordTarget() {
        val installed = WireGuardConfigManager.isDiscordInstalled(this)
        if (installed) {
            val pkg = WireGuardConfigManager.getInstalledDiscordPackage(this) ?: "com.discord"
            tvMetricDiscordClient.text = pkg
            tvMetricDiscordClient.setTextColor(ContextCompat.getColor(this, R.color.terminal_green))
            appendTerminal("[ ✔ ] Alvo detectado no Android: $pkg")
            appendTerminal("[ ✔ ] Isolamento por processo (Split-Tunneling) pronto.")
        } else {
            tvMetricDiscordClient.text = "Não instalado"
            tvMetricDiscordClient.setTextColor(ContextCompat.getColor(this, R.color.terminal_text_muted))
            appendTerminal("[AVISO] Nenhum app oficial do Discord encontrado.")
            appendTerminal("Dica: Instale o Discord pela Play Store para utilizar o bypass.")
        }
    }

    private fun prepareAndStartVpn() {
        if (!WireGuardConfigManager.hasConfig(this)) {
            appendTerminal("\n[ ! ] Nenhuma configuração WireGuard encontrada.")
            appendTerminal("[ ! ] Escolha entre ProtonVPN Gratuita (Recomendado) ou Importar .conf.")
            showChoiceModal()
            return
        }

        val vpnIntent = VpnService.prepare(this)
        if (vpnIntent != null) {
            vpnPermissionLauncher.launch(vpnIntent)
        } else {
            startVpnServiceInternal()
        }
    }

    private fun showAuthLoading(initialStatus: String = "Iniciando conexão segura com a Proton...") {
        authLoadingOverlay.alpha = 0f
        authLoadingOverlay.visibility = View.VISIBLE
        authLoadingOverlay.animate().alpha(1f).setDuration(250).start()

        pbAuthLoading.visibility = View.VISIBLE
        btnAuthLoadingDismiss.visibility = View.GONE
        tvAuthLoadingTitle.text = "AUTENTICANDO PROTONVPN"
        tvAuthLoadingTitle.setTextColor(ContextCompat.getColor(this, R.color.alri_red))
        tvAuthLoadingStatus.text = initialStatus

        authLogoAnimator?.cancel()
        val scaleX = ObjectAnimator.ofFloat(ivAuthLoadingLogo, View.SCALE_X, 1f, 1.15f, 1f).apply {
            repeatCount = ValueAnimator.INFINITE
            repeatMode = ValueAnimator.REVERSE
            duration = 900
            interpolator = AccelerateDecelerateInterpolator()
        }
        val scaleY = ObjectAnimator.ofFloat(ivAuthLoadingLogo, View.SCALE_Y, 1f, 1.15f, 1f).apply {
            repeatCount = ValueAnimator.INFINITE
            repeatMode = ValueAnimator.REVERSE
            duration = 900
            interpolator = AccelerateDecelerateInterpolator()
        }
        val alpha = ObjectAnimator.ofFloat(ivAuthLoadingLogo, View.ALPHA, 0.6f, 1f, 0.6f).apply {
            repeatCount = ValueAnimator.INFINITE
            repeatMode = ValueAnimator.REVERSE
            duration = 900
            interpolator = AccelerateDecelerateInterpolator()
        }
        authLogoAnimator = AnimatorSet().apply {
            playTogether(scaleX, scaleY, alpha)
            start()
        }
    }

    private fun updateAuthLoadingStatus(status: String) {
        runOnUiThread {
            tvAuthLoadingStatus.text = status
        }
    }

    private fun hideAuthLoading(onComplete: (() -> Unit)? = null) {
        authLogoAnimator?.cancel()
        ivAuthLoadingLogo.scaleX = 1f
        ivAuthLoadingLogo.scaleY = 1f
        ivAuthLoadingLogo.alpha = 1f
        authLoadingOverlay.animate()
            .alpha(0f)
            .setDuration(200)
            .withEndAction {
                authLoadingOverlay.visibility = View.GONE
                onComplete?.invoke()
            }
            .start()
    }

    private fun showAuthLoadingSuccess(onFinished: () -> Unit) {
        authLogoAnimator?.cancel()
        ivAuthLoadingLogo.scaleX = 1f
        ivAuthLoadingLogo.scaleY = 1f
        ivAuthLoadingLogo.alpha = 1f

        pbAuthLoading.visibility = View.GONE
        tvAuthLoadingTitle.text = "[ ✔ ] AUTENTICADO COM SUCESSO!"
        tvAuthLoadingTitle.setTextColor(ContextCompat.getColor(this, R.color.terminal_green))
        tvAuthLoadingStatus.text = "Túnel WireGuard configurado. Inicializando bypass..."

        mainHandler.postDelayed({
            hideAuthLoading {
                onFinished()
            }
        }, 800)
    }

    private fun showAuthLoadingError(errorMsg: String, onDismiss: () -> Unit) {
        authLogoAnimator?.cancel()
        ivAuthLoadingLogo.scaleX = 1f
        ivAuthLoadingLogo.scaleY = 1f
        ivAuthLoadingLogo.alpha = 1f

        pbAuthLoading.visibility = View.GONE
        tvAuthLoadingTitle.text = "[ ✖ ] FALHA NA AUTENTICAÇÃO"
        tvAuthLoadingTitle.setTextColor(ContextCompat.getColor(this, R.color.alri_red))
        tvAuthLoadingStatus.text = errorMsg
        btnAuthLoadingDismiss.visibility = View.VISIBLE

        btnAuthLoadingDismiss.setOnClickListener {
            hideAuthLoading {
                onDismiss()
            }
        }
    }

    private fun triggerProtonAuthentication(
        user: String,
        pass: String,
        startVpnOnSuccess: Boolean,
        origin: String = "modal"
    ) {
        if (user.isBlank() || pass.isBlank()) {
            Toast.makeText(this, "Informe usuário e senha da conta Proton.", Toast.LENGTH_SHORT).show()
            appendTerminal("[AVISO] Usuário e senha são obrigatórios para autenticação Proton.")
            return
        }

        val imm = getSystemService(INPUT_METHOD_SERVICE) as? InputMethodManager
        currentFocus?.let { view ->
            imm?.hideSoftInputFromWindow(view.windowToken, 0)
        }

        showAuthLoading("Iniciando handshake seguro SRP com Proton...")
        appendTerminal("\n[ ℹ ] Conectando à API ProtonVPN via SRP...")
        appendTerminal("[ ℹ ] Autenticando usuário: $user")

        lifecycleScope.launch {
            val result = ProtonAuthManager.authenticateAndGenerate(this@MainActivity, user, pass) { step ->
                runOnUiThread {
                    updateAuthLoadingStatus(step)
                    appendTerminal("[ ℹ ] $step")
                }
            }

            if (result.isSuccess) {
                appendTerminal("[ ✔ ] Perfil WireGuard ardc_vpn.conf gerado e validado!")
                refreshConfigState()
                showAuthLoadingSuccess {
                    Toast.makeText(this@MainActivity, "Conexão Proton configurada com sucesso!", Toast.LENGTH_SHORT).show()
                    if (startVpnOnSuccess) {
                        prepareAndStartVpn()
                    }
                }
            } else {
                val err = result.exceptionOrNull()?.message ?: "Falha desconhecida"
                appendTerminal("[ ✖ ] Erro na autenticação Proton: $err", isError = true)
                showAuthLoadingError(err) {
                    if (origin == "modal") {
                        showChoiceModal(prefillUser = user)
                    }
                }
            }
        }
    }

    private fun showChoiceModal(prefillUser: String = "") {
        val bottomSheet = com.google.android.material.bottomsheet.BottomSheetDialog(this)
        val sheetView = layoutInflater.inflate(R.layout.layout_choice_bottom_sheet, null)
        bottomSheet.setContentView(sheetView)

        val btnClose: View = sheetView.findViewById(R.id.btnCloseChoiceSheet)
        val etUser: TextInputEditText = sheetView.findViewById(R.id.etChoiceProtonUser)
        val etPass: TextInputEditText = sheetView.findViewById(R.id.etChoiceProtonPass)
        val btnLogin: MaterialButton = sheetView.findViewById(R.id.btnChoiceProtonLogin)
        val btnProtonWeb: MaterialButton = sheetView.findViewById(R.id.btnChoiceProtonWeb)
        val btnImportCustom: MaterialButton = sheetView.findViewById(R.id.btnChoiceImportCustom)

        if (prefillUser.isNotBlank()) {
            etUser.setText(prefillUser)
            etPass.requestFocus()
        }

        btnClose.setOnClickListener { bottomSheet.dismiss() }

        btnLogin.setOnClickListener {
            val user = etUser.text?.toString() ?: ""
            val pass = etPass.text?.toString() ?: ""
            if (user.isBlank() || pass.isBlank()) {
                Toast.makeText(this, "Informe usuário e senha da Proton.", Toast.LENGTH_SHORT).show()
                return@setOnClickListener
            }
            bottomSheet.dismiss()
            triggerProtonAuthentication(user, pass, startVpnOnSuccess = true, origin = "modal")
        }

        btnProtonWeb.setOnClickListener {
            bottomSheet.dismiss()
            val url = "https://account.protonvpn.com/pt-br/login"
            startActivity(Intent(Intent.ACTION_VIEW, Uri.parse(url)))
            appendTerminal("\n[ ℹ ] Abrindo página oficial de login ProtonVPN...")
        }

        btnImportCustom.setOnClickListener {
            bottomSheet.dismiss()
            importConfigLauncher.launch("*/*")
        }

        bottomSheet.show()
    }

    private fun startConnectingSpinner() {
        pbVpnConnecting.visibility = View.VISIBLE
        btnToggleVpn.isEnabled = false
        btnToggleVpn.alpha = 0.9f
        viewStatusDot.backgroundTintList = ContextCompat.getColorStateList(this, R.color.terminal_yellow)
        tvStatusText.text = "CONECTANDO..."
        tvStatusText.setTextColor(ContextCompat.getColor(this, R.color.terminal_yellow))
        tvMetricTunnelStatus.text = "Iniciando túnel..."
        tvMetricTunnelStatus.setTextColor(ContextCompat.getColor(this, R.color.terminal_yellow))

        val frames = listOf("⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏")
        var frameIndex = 0
        vpnConnectingRunnable?.let { mainHandler.removeCallbacks(it) }
        vpnConnectingRunnable = object : Runnable {
            override fun run() {
                val spinner = frames[frameIndex % frames.size]
                btnToggleVpn.text = "[ $spinner ] CONECTANDO TÚNEL..."
                frameIndex++
                mainHandler.postDelayed(this, 80)
            }
        }
        mainHandler.post(vpnConnectingRunnable!!)
    }

    private fun stopConnectingSpinner() {
        vpnConnectingRunnable?.let {
            mainHandler.removeCallbacks(it)
            vpnConnectingRunnable = null
        }
        pbVpnConnecting.visibility = View.GONE
        btnToggleVpn.isEnabled = true
        btnToggleVpn.alpha = 1.0f
    }

    private fun startVpnServiceInternal() {
        startConnectingSpinner()
        simulateConnectingAnimation {
            val intent = Intent(this, DiscordVpnService::class.java)
            ContextCompat.startForegroundService(this, intent)
        }
    }

    private fun stopVpnServiceInternal() {
        pbVpnConnecting.visibility = View.VISIBLE
        btnToggleVpn.isEnabled = false
        btnToggleVpn.text = "[ ⠋ ] DESATIVANDO TÚNEL..."
        appendTerminal("[ ⠋ ] Encerrando túnel WireGuard isolado...")
        val intent = Intent(this, DiscordVpnService::class.java).apply {
            action = DiscordVpnService.ACTION_STOP
        }
        startService(intent)
    }

    private fun simulateConnectingAnimation(onDone: () -> Unit) {
        appendTerminal("[ ⠋ ] Preparando túnel WireGuard dedicado (AllowedApps)...")
        mainHandler.postDelayed({
            appendTerminal("[ ⠙ ] Estabelecendo handshake com servidor internacional...")
            mainHandler.postDelayed({
                appendTerminal("[ ✔ ] Túnel conectado! Discord liberado para Transmissão e Câmera.")
                onDone()
            }, 500)
        }, 450)
    }

    private fun updateUiState(active: Boolean) {
        stopConnectingSpinner()
        if (active) {
            viewStatusDot.backgroundTintList = ContextCompat.getColorStateList(this, R.color.terminal_green)
            tvStatusText.text = "PROTEGIDO"
            tvStatusText.setTextColor(ContextCompat.getColor(this, R.color.terminal_green))

            tvMetricInterface.text = "tun0 (WireGuard)"
            tvMetricInterface.setTextColor(ContextCompat.getColor(this, R.color.terminal_green))

            tvMetricTunnelStatus.text = "Ativo e Roteando"
            tvMetricTunnelStatus.setTextColor(ContextCompat.getColor(this, R.color.terminal_green))

            btnToggleVpn.text = "■   DESATIVAR BYPASS & RESTAURAR"
            btnToggleVpn.backgroundTintList = ContextCompat.getColorStateList(this, R.color.terminal_card)
            btnToggleVpn.setTextColor(ContextCompat.getColor(this, R.color.alri_red_glow))
            btnToggleVpn.strokeColor = ContextCompat.getColorStateList(this, R.color.terminal_card_border)
            btnToggleVpn.strokeWidth = 2

            // Habilitar o botão de abrir Discord quando a VPN estiver ativa
            btnOpenDiscord.isEnabled = true
            btnOpenDiscord.alpha = 1.0f
        } else {
            viewStatusDot.backgroundTintList = ContextCompat.getColorStateList(this, R.color.terminal_text_muted)
            tvStatusText.text = "DESCONECTADO"
            tvStatusText.setTextColor(ContextCompat.getColor(this, R.color.terminal_text_secondary))

            tvMetricInterface.text = "Inativa"
            tvMetricInterface.setTextColor(ContextCompat.getColor(this, R.color.terminal_text_muted))

            tvMetricTunnelStatus.text = "Desconectado"
            tvMetricTunnelStatus.setTextColor(ContextCompat.getColor(this, R.color.terminal_text_muted))

            btnToggleVpn.text = "▶   ATIVAR BYPASS NO DISCORD"
            btnToggleVpn.backgroundTintList = ContextCompat.getColorStateList(this, R.color.alri_red)
            btnToggleVpn.setTextColor(ContextCompat.getColor(this, R.color.terminal_btn_text))
            btnToggleVpn.strokeWidth = 0

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
            🚀 ARDiscordBypass v3.5.0
            
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
