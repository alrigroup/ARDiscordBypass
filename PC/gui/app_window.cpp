#include "app_window.h"
#include "widgets.h"
#include "style.h"
#include <cstring>
#include <cmath>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#include <commdlg.h>
#include <shellapi.h>
#endif

namespace GUI {

static std::string openNativeFileDialog() {
#ifdef _WIN32
  wchar_t szFile[MAX_PATH] = {0};
  OPENFILENAMEW ofn = {0};
  ofn.lStructSize = sizeof(ofn);
  ofn.lpstrFile = szFile;
  ofn.nMaxFile = sizeof(szFile) / sizeof(wchar_t);
  ofn.lpstrFilter = L"WireGuard Config (*.conf)\0*.conf\0All Files (*.*)\0*.*\0";
  ofn.nFilterIndex = 1;
  ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
  if (GetOpenFileNameW(&ofn)) {
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, szFile, -1, NULL, 0, NULL, NULL);
    std::string str(size_needed - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, szFile, -1, &str[0], size_needed, NULL, NULL);
    return str;
  }
  return "";
#else
  std::string out = runCommand("zenity --file-selection --title=\"Selecione o arquivo .conf WireGuard\" --file-filter=\"*.conf\" 2>/dev/null");
  if (!out.empty()) {
    out.erase(out.find_last_not_of(" \t\r\n") + 1);
    return out;
  }
  out = runCommand("kdialog --getopenfilename . \"*.conf\" 2>/dev/null");
  if (!out.empty()) {
    out.erase(out.find_last_not_of(" \t\r\n") + 1);
    return out;
  }
  return "";
#endif
}

AppWindow::AppWindow() {
  m_status = Core::Service::instance().queryStatus();
  m_lastProbe = std::chrono::steady_clock::now();

  if (fs::is_regular_file("ardc_vpn.conf")) {
    m_configMode = 0;
  }
}

AppWindow::~AppWindow() {
  if (m_future.valid()) {
    m_future.wait();
  }
}

void AppWindow::setDarkMode(bool dark) {
  m_darkMode = dark;
  applyTheme(m_darkMode);
}

void AppWindow::render() {
  if (m_splashStartTime == 0.0f) {
    m_splashStartTime = static_cast<float>(ImGui::GetTime());
  }

  ImGui::SetNextWindowPos(ImVec2(0, 0));
  ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);

  ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                           ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;

  ImGui::Begin("MainWindow", nullptr, flags);

  // Animação de subida suave do conteúdo na transição de Unlock
  if (!m_splashDone) {
    float elapsed = static_cast<float>(ImGui::GetTime()) - m_splashStartTime;
    if (elapsed > 1.1f) {
      float t = (elapsed - 1.1f) / 0.7f;
      if (t > 1.0f) t = 1.0f;
      float ease = 1.0f - std::pow(1.0f - t, 3.0f);
      float yOffset = (1.0f - ease) * 50.0f;
      ImGui::SetCursorPosY(ImGui::GetCursorPosY() + yOffset);
    }
  }

  // Polling de status periódico (a cada 2 segundos) sem travar a interface
  auto now = std::chrono::steady_clock::now();
  if (!m_isWorking && std::chrono::duration_cast<std::chrono::seconds>(now - m_lastProbe).count() >= 2) {
    std::string conf = (m_configMode == 1 && m_customPathBuf[0] != '\0') ? m_customPathBuf : "ardc_vpn.conf";
    m_status = Core::Service::instance().queryStatus(conf);
    m_lastProbe = now;
  }

  renderHeader();

  ImGui::Spacing();
  renderNavigation();
  ImGui::Spacing();

  float tabElapsed = static_cast<float>(ImGui::GetTime()) - m_tabAnimStartTime;
  float tabProgress = (m_tabAnimStartTime > 0.0f) ? std::min(1.0f, tabElapsed / 0.20f) : 1.0f;
  float tabEase = 1.0f - std::pow(1.0f - tabProgress, 2.0f);
  float tabYOffset = (1.0f - tabEase) * 12.0f;

  ImGui::PushStyleVar(ImGuiStyleVar_Alpha, tabEase);
  ImGui::SetCursorPosY(ImGui::GetCursorPosY() + tabYOffset);

  switch (m_activeTab) {
    case 0: renderDashboard(); break;
    case 1: renderConfigTab(); break;
    case 2: renderLogsTab(); break;
    case 3: renderAboutTab(); break;
  }

  ImGui::PopStyleVar();

  if (m_isAuthWorking || !m_authError.empty() || (m_isWorking && !m_isConnecting)) {
    renderLoadingOverlay();
  }

  // Overlay de Splash / Animação de Abertura Unlock
  if (!m_splashDone) {
    renderSplashOverlay();
  }

  if (m_showSetupModal) {
    renderSetupModal();
  }

  if (m_showNotesModal) {
    renderNotesModal();
  }

  if (m_showCreditsModal) {
    renderCreditsModal();
  }

  ImGui::End();
}

void AppWindow::renderHeader() {
  const ThemeColors &th = getTheme();
  ImDrawList *drawList = ImGui::GetWindowDrawList();

  ImGui::BeginGroup();

  // Renderização da Logo Oficial da ALRI
  if (m_logoTexId != 0) {
    if (m_splashDone) {
      ImGui::Image(static_cast<ImTextureID>(m_logoTexId), ImVec2(42, 42));
    } else {
      ImGui::Dummy(ImVec2(42, 42)); // Reserva espaço enquanto a splash transiciona
    }
    ImGui::SameLine(0.0f, 12.0f);
  }

  // Títulos
  ImGui::BeginGroup();
  ImGui::TextColored(th.alriRed, "ARDiscordBypass");
  ImGui::SameLine(0.0f, 6.0f);
  ImGui::TextColored(th.textMuted, "v3.5.0");
  ImGui::TextColored(th.textSecondary, "ALRI GROUP • Split-Tunneling WireGuard");
  ImGui::EndGroup();

  ImGui::EndGroup();

  // Lado direito: Status Pill + Botão de Alternância de Tema
  float rightAreaWidth = 290.0f;
  float rightX = ImGui::GetWindowWidth() - rightAreaWidth - 16.0f;
  if (rightX > 320.0f) {
    ImGui::SameLine(rightX);
  } else {
    ImGui::Spacing();
  }

  ImGui::BeginGroup();

  // 1. Badge de Status com Pulsing Dot
  ImVec2 statusPillPos = ImGui::GetCursorScreenPos();
  float pillW = 160.0f;
  float pillH = 34.0f;

  ImU32 pillBg;
  ImU32 pillBorder;
  ImU32 dotColor;
  const char* pillText;
  ImVec4 textColor;

  if (m_isConnecting) {
    pillBg = IM_COL32(255, 214, 0, 30);
    pillBorder = IM_COL32(255, 214, 0, 140);
    dotColor = IM_COL32(255, 214, 0, 255);
    pillText = m_isStopping ? "DESCONECTANDO..." : "CONECTANDO...";
    textColor = th.terminalYellow;
  } else if (m_status.isTunnelUp) {
    pillBg = IM_COL32(0, 230, 118, 30);
    pillBorder = IM_COL32(0, 230, 118, 120);
    dotColor = IM_COL32(0, 230, 118, 255);
    pillText = "PROTEGIDO";
    textColor = th.terminalGreen;
  } else {
    pillBg = th.isDark ? IM_COL32(25, 25, 25, 255) : IM_COL32(230, 235, 240, 255);
    pillBorder = th.isDark ? IM_COL32(50, 50, 50, 255) : IM_COL32(200, 205, 215, 255);
    dotColor = IM_COL32(148, 163, 184, 255);
    pillText = "DESCONECTADO";
    textColor = th.textMuted;
  }

  drawList->AddRectFilled(statusPillPos, ImVec2(statusPillPos.x + pillW, statusPillPos.y + pillH), pillBg, 17.0f);
  drawList->AddRect(statusPillPos, ImVec2(statusPillPos.x + pillW, statusPillPos.y + pillH), pillBorder, 17.0f, 0, 1.0f);

  ImVec2 dotCenter = ImVec2(statusPillPos.x + 18.0f, statusPillPos.y + 17.0f);
  DrawPulsingStatusDot(drawList, dotCenter, 4.5f, dotColor, m_isConnecting || m_status.isTunnelUp);

  ImGui::SetCursorScreenPos(ImVec2(statusPillPos.x + 32.0f, statusPillPos.y + 8.0f));
  ImGui::TextColored(textColor, "%s", pillText);

  // 2. Botão de Alternância de Tema (Light / Dark)
  ImGui::SetCursorScreenPos(ImVec2(statusPillPos.x + pillW + 10.0f, statusPillPos.y));
  ImVec2 themeBtnPos = ImGui::GetCursorScreenPos();
  float themeBtnW = 105.0f;

  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 17.0f);
  ImGui::PushStyleColor(ImGuiCol_Button, th.card);
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, th.cardBorder);
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, th.surface);

  std::string themeLabel = m_darkMode ? "   ESCURO" : "   CLARO";
  if (ImGui::Button(themeLabel.c_str(), ImVec2(themeBtnW, pillH))) {
    setDarkMode(!m_darkMode);
  }

  ImVec2 iconCenter = ImVec2(themeBtnPos.x + 18.0f, themeBtnPos.y + 17.0f);
  ImU32 iconColor = m_darkMode ? IM_COL32(255, 214, 0, 255) : IM_COL32(229, 9, 20, 255);
  DrawThemeIcon(drawList, iconCenter, 6.0f, m_darkMode, iconColor);

  ImGui::PopStyleColor(3);
  ImGui::PopStyleVar();

  ImGui::EndGroup();

  ImGui::Spacing();
  ImGui::Separator();
}

void AppWindow::renderNavigation() {
  const ThemeColors &th = getTheme();
  const char *tabs[] = {"DASHBOARD", "CONFIGURACOES", "LOGS", "SOBRE"};
  const float tabWidths[] = {130.0f, 150.0f, 110.0f, 100.0f};

  for (int i = 0; i < 4; ++i) {
    if (i > 0) ImGui::SameLine(0.0f, 8.0f);

    bool isActive = (m_activeTab == i);
    if (isActive) {
      ImGui::PushStyleColor(ImGuiCol_Button, th.alriRed);
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered, th.alriRedGlow);
      ImGui::PushStyleColor(ImGuiCol_ButtonActive, th.alriRedDark);
      ImGui::PushStyleColor(ImGuiCol_Text, th.btnText);
    } else {
      ImGui::PushStyleColor(ImGuiCol_Button, th.card);
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered, th.cardBorder);
      ImGui::PushStyleColor(ImGuiCol_ButtonActive, th.surface);
      ImGui::PushStyleColor(ImGuiCol_Text, th.textSecondary);
    }

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
    if (ImGui::Button(tabs[i], ImVec2(tabWidths[i], 36.0f))) {
      if (m_activeTab != i) {
        m_activeTab = i;
        m_tabAnimStartTime = static_cast<float>(ImGui::GetTime());
      }
    }
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(4);
  }
}

void AppWindow::renderDashboard() {
  const ThemeColors &th = getTheme();
  ImDrawList *drawList = ImGui::GetWindowDrawList();

  // Card de Métricas e Status (Estilo Box Terminal Android)
  ImGui::BeginChild("DashboardCards", ImVec2(0, 200), true);

  ImGui::TextColored(th.textMuted, "[ STATUS DA CONEXAO & METRICAS ]");
  ImGui::Separator();
  ImGui::Spacing();

  ImGui::Columns(2, "StatusColumns", false);
  ImGui::SetColumnWidth(0, 310.0f);

  ImGui::TextColored(th.textSecondary, "Interface WireGuard:");
  ImGui::SameLine(180.0f);
  if (m_isConnecting) {
    ImGui::TextColored(th.terminalYellow, "%s", m_isStopping ? "Encerrando..." : "Iniciando tunel...");
  } else if (m_status.isTunnelUp) {
    ImGui::TextColored(th.terminalGreen, "wg-ardc (Ativa)");
  } else {
    ImGui::TextColored(th.textMuted, "Inativa");
  }

  ImGui::TextColored(th.textSecondary, "IP Egress Internacional:");
  ImGui::SameLine(180.0f);
  if (m_status.isTunnelUp && !m_status.egressIp.empty() && m_status.egressIp != "--") {
    ImGui::TextColored(ImVec4(0.0f, 0.82f, 1.0f, 1.0f), "%s (%s)", m_status.egressIp.c_str(), m_status.egressCountry.c_str());
  } else {
    ImGui::TextDisabled("--");
  }

  ImGui::TextColored(th.textSecondary, "Ultimo Handshake:");
  ImGui::SameLine(180.0f);
  if (m_isConnecting) {
    ImGui::TextColored(th.terminalYellow, "Negociando...");
  } else {
    ImGui::TextColored(th.textPrimary, "%s", m_status.lastHandshake.c_str());
  }

  ImGui::NextColumn();

  ImGui::TextColored(th.textSecondary, "Cliente Discord:");
  ImGui::SameLine(150.0f);
  ImGui::TextColored(th.textPrimary, "%s", m_status.discordClient.c_str());

  ImGui::TextColored(th.textSecondary, "Status do Processo:");
  ImGui::SameLine(150.0f);
  if (m_isConnecting) {
    ImGui::TextColored(th.terminalYellow, "%s", m_isStopping ? "Restaurando..." : "Preparando...");
  } else if (m_status.isDiscordActive) {
    ImGui::TextColored(th.terminalGreen, "Em Execucao");
  } else {
    ImGui::TextDisabled("Fechado");
  }

  ImGui::TextColored(th.textSecondary, "Latencia (Ping):");
  ImGui::SameLine(150.0f);
  ImGui::TextColored(th.textPrimary, "%s", m_status.pingMs.c_str());

  ImGui::Columns(1);
  ImGui::EndChild();

  ImGui::Spacing();

  // Alerta quando não há configuração detectada
  if (!m_status.isTunnelUp && !m_isConnecting && !hasValidConfig()) {
    ImGui::PushStyleColor(ImGuiCol_ChildBg, th.surface);
    ImGui::BeginChild("ConfigAlertBox", ImVec2(0, 46), true);
    ImGui::TextColored(th.terminalYellow, "[!] Nenhuma configuracao salva.");
    ImGui::SameLine(180.0f);
    ImGui::TextColored(th.textMuted, "Proton Gratis (Recomendado) ou Perfil .conf");
    ImGui::SameLine(ImGui::GetWindowWidth() - 170.0f);
    if (ImGui::Button(" Escolher Perfil... ", ImVec2(150, 26))) {
      m_showSetupModal = true;
    }
    ImGui::EndChild();
    ImGui::PopStyleColor();
    ImGui::Spacing();
  }

  // Botão Principal de Ação (100% ALRI RED - Sem botões verdes, idêntico ao APK!)
  ImVec2 btnPos = ImGui::GetCursorScreenPos();
  float btnHeight = 56.0f;
  float btnWidth = ImGui::GetContentRegionAvail().x;

  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);

  if (m_isConnecting) {
    const char *frames[] = {"⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"};
    int frameIdx = static_cast<int>(ImGui::GetTime() * 12.0) % 10;
    std::string btnLabel = m_isStopping
      ? (std::string("       [ ") + frames[frameIdx] + " ] DESATIVANDO TUNEL...")
      : (std::string("       [ ") + frames[frameIdx] + " ] CONECTANDO TUNEL ISOLADO...");

    ImGui::PushStyleColor(ImGuiCol_Button, th.card);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, th.card);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, th.card);
    ImGui::PushStyleColor(ImGuiCol_Text, th.terminalYellow);

    ImGui::Button(btnLabel.c_str(), ImVec2(-1, btnHeight));

    ImGui::PopStyleColor(4);

    // Barra de progresso horizontal indeterminada vermelha (igual ao APK!)
    ImVec2 barMin = ImVec2(btnPos.x, btnPos.y + btnHeight + 5.0f);
    ImVec2 barMax = ImVec2(btnPos.x + btnWidth, btnPos.y + btnHeight + 9.0f);
    DrawIndeterminateProgressBar(drawList, barMin, barMax, ImGui::GetColorU32(th.surface), ImGui::GetColorU32(th.alriRed));
    ImGui::Dummy(ImVec2(0, 10.0f));
  } else if (!m_status.isTunnelUp) {
    ImGui::PushStyleColor(ImGuiCol_Button, th.alriRed);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, th.alriRedGlow);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, th.alriRedDark);
    ImGui::PushStyleColor(ImGuiCol_Text, th.btnText);

    if (ImGui::Button("       ATIVAR BYPASS & ABRIR DISCORD", ImVec2(-1, btnHeight)) && !m_isWorking) {
      triggerStart();
    }

    ImVec2 iconPos = ImVec2(btnPos.x + 24.0f, btnPos.y + (btnHeight - 16.0f) * 0.5f);
    DrawPlayIcon(drawList, iconPos, 16.0f, IM_COL32_WHITE);

    ImGui::PopStyleColor(4);
  } else {
    ImGui::PushStyleColor(ImGuiCol_Button, th.card);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, th.cardBorder);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, th.surface);
    ImGui::PushStyleColor(ImGuiCol_Text, th.alriRedGlow);

    if (ImGui::Button("       DESATIVAR BYPASS & RESTAURAR REDE", ImVec2(-1, btnHeight)) && !m_isWorking) {
      triggerStop();
    }

    ImVec2 iconPos = ImVec2(btnPos.x + 24.0f, btnPos.y + (btnHeight - 16.0f) * 0.5f);
    DrawStopIcon(drawList, iconPos, 16.0f, IM_COL32(255, 30, 39, 255));

    ImGui::PopStyleColor(4);
  }

  ImGui::PopStyleVar();

  ImGui::Spacing();
  ImGui::PushStyleColor(ImGuiCol_ChildBg, th.surface);
  ImGui::BeginChild("SplitTunnelNotice", ImVec2(0, 82.0f), true, ImGuiWindowFlags_None);
  ImGui::TextColored(th.terminalGreen, "[ VPN SELETIVA // SPLIT-TUNNELING EXCLUSIVO AO DISCORD ]");
  ImGui::Spacing();
  ImGui::TextColored(th.textPrimary, "Este aplicativo isola e protege UNICA E EXCLUSIVAMENTE o trafego do Discord.");
  ImGui::TextColored(th.textMuted, "Seus jogos, navegadores, bancos, downloads e outros apps continuam 100%% na sua conexao normal sem alteracao de rota ou aumento de ping.");
  ImGui::EndChild();
  ImGui::PopStyleColor();
}

void AppWindow::renderConfigTab() {
  const ThemeColors &th = getTheme();
  ImGui::BeginChild("ConfigContent", ImVec2(0, 0), true);

  ImGui::TextColored(th.textMuted, "[ SELECAO DE ROTA WIREGUARD ]");
  ImGui::Separator();
  ImGui::Spacing();

  ImGui::RadioButton("ProtonVPN Automatico (Internacional Gratuito / Ilimitado)", &m_configMode, 0);
  if (m_configMode == 0) {
    ImGui::Indent(24.0f);
    bool hasSession = Core::Service::instance().hasSavedSession();
    if (hasSession) {
      ImGui::TextColored(th.terminalGreen, "[OK] Sessao salva detectada (~/.protonvpn-session.json)");
      ImGui::TextColored(th.textMuted, "O aplicativo renova o tunel automaticamente sem pedir credenciais.");
    } else {
      ImGui::TextColored(th.terminalYellow, "[!] Nenhuma sessao salva. Informe suas credenciais gratuitas da Proton:");
    }

    ImGui::Spacing();
    ImGui::TextColored(th.textSecondary, "Usuario Proton:");
    ImGui::InputText("##protonUser", m_protonUserBuf, sizeof(m_protonUserBuf));

    ImGui::TextColored(th.textSecondary, "Senha Proton:");
    ImGuiInputTextFlags passFlags = m_hidePassword ? ImGuiInputTextFlags_Password : 0;
    ImGui::InputText("##protonPass", m_protonPassBuf, sizeof(m_protonPassBuf), passFlags);
    ImGui::SameLine();
    if (ImGui::Button(m_hidePassword ? "Mostrar" : "Ocultar")) {
      m_hidePassword = !m_hidePassword;
    }

    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Button, th.alriRed);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, th.alriRedGlow);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, th.alriRedDark);
    if (ImGui::Button("  Autenticar & Gerar Perfil  ", ImVec2(220, 36)) && !m_isWorking) {
      triggerProtonLogin();
    }
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    if (ImGui::Button("Criar Conta / Esqueceu a Senha (Web)", ImVec2(270, 36))) {
      openBrowser("https://account.protonvpn.com/pt-br/login");
    }
    ImGui::Unindent(24.0f);
  }

  ImGui::Spacing();
  ImGui::Separator();
  ImGui::Spacing();

  ImGui::RadioButton("Perfil WireGuard Customizado (.conf pessoal / VPS)", &m_configMode, 1);
  if (m_configMode == 1) {
    ImGui::Indent(24.0f);
    ImGui::TextColored(th.textSecondary, "Caminho do arquivo .conf:");
    ImGui::PushItemWidth(ImGui::GetWindowWidth() - 170.0f);
    ImGui::InputText("##customPath", m_customPathBuf, sizeof(m_customPathBuf));
    ImGui::PopItemWidth();
    ImGui::SameLine();
    if (ImGui::Button(" Procurar... ") && !m_isWorking) {
      pickConfigFile();
    }
    ImGui::TextColored(th.textMuted, "Compativel com perfis WireGuard de VPS proprias, Mullvad, WARP, etc.");
    ImGui::Unindent(24.0f);
  }

  ImGui::Spacing();
  ImGui::Separator();
  ImGui::Spacing();

  // Seção de Gerenciamento de Dados e Limpeza Total
  ImGui::TextColored(th.textMuted, "[ GERENCIAMENTO DE DADOS & LIMPEZA DE CONFIGURACOES ]");
  ImGui::Separator();
  ImGui::Spacing();

  bool hasConf = fs::is_regular_file("ardc_vpn.conf");
  bool hasSess = Core::Service::instance().hasSavedSession();

  ImGui::TextColored(th.textSecondary, "Perfil WireGuard Local (ardc_vpn.conf):");
  ImGui::SameLine(340.0f);
  if (hasConf) ImGui::TextColored(th.terminalGreen, "Presente no disco");
  else ImGui::TextColored(th.textMuted, "Ausente");

  ImGui::TextColored(th.textSecondary, "Sessao de Login Proton (.protonvpn-session.json):");
  ImGui::SameLine(340.0f);
  if (hasSess) ImGui::TextColored(th.terminalGreen, "Sessao ativa salva");
  else ImGui::TextColored(th.textMuted, "Nenhuma sessao");

  ImGui::Spacing();
  ImGui::TextWrapped("Para apagar todas as credenciais de login salvas, remover o perfil WireGuard e restaurar o aplicativo ao estado original de fabrica:");
  ImGui::Spacing();

  ImGui::PushStyleColor(ImGuiCol_Button, th.alriRed);
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, th.alriRedGlow);
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, th.alriRedDark);
  if (ImGui::Button("  LIMPAR TODAS AS CONFIGURACOES E DADOS SALVOS  ", ImVec2(370, 38)) && !m_isWorking) {
    triggerClearAll();
  }
  ImGui::PopStyleColor(3);

  ImGui::EndChild();
}

void AppWindow::renderLogsTab() {
  const ThemeColors &th = getTheme();
  ImGui::BeginChild("LogsContent", ImVec2(0, 0), true);

  if (ImGui::Button("  Limpar Logs  ")) {
    Core::Service::instance().clearLogs();
  }
  ImGui::SameLine();
  ImGui::TextColored(th.textMuted, "alri@ardcb:~$ tail -f /var/log/ardcb_tunnel.log");

  ImGui::Separator();
  ImGui::Spacing();

  // Caixa de logs estilizada igual ao terminal retro-moderno do app Android
  auto logs = Core::Service::instance().getLogs();
  for (const auto &entry : logs) {
    ImVec4 col = th.textPrimary;
    if (entry.level == 1) col = th.terminalGreen;
    else if (entry.level == 2) col = th.terminalYellow;
    else if (entry.level == 3) col = th.alriRed;

    ImGui::TextColored(th.textMuted, "%s", entry.timeStr.c_str());
    ImGui::SameLine();
    ImGui::TextColored(col, "%s", entry.message.c_str());
  }

  // Cursor piscante de terminal
  bool showCursor = (static_cast<int>(ImGui::GetTime() * 2.0) % 2 == 0);
  if (showCursor) {
    ImGui::TextColored(th.terminalGreen, "> _");
  } else {
    ImGui::TextColored(th.terminalGreen, ">");
  }

  if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
    ImGui::SetScrollHereY(1.0f);
  }

  ImGui::EndChild();
}

void AppWindow::renderAboutTab() {
  const ThemeColors &th = getTheme();
  ImGui::BeginChild("AboutContent", ImVec2(0, 0), true);

  ImGui::TextColored(th.alriRed, "ARDiscordBypass — ALRI GROUP");
  ImGui::TextColored(th.textPrimary, "Versao: 3.5.0 (Desktop GUI + CLI)");
  ImGui::TextColored(th.textMuted, "Licenca: ARGLFU Version 2 (2026)");
  ImGui::Separator();
  ImGui::Spacing();

  ImGui::TextWrapped("O ARDiscordBypass e uma solucao de alto desempenho desenvolvida para liberar transmissoes de tela e camera no Discord no Brasil com zero impacto em jogos e navegadores.");
  ImGui::Spacing();

  ImGui::BulletText("Isolamento estrito por processo (Split-Tunneling)");
  ImGui::BulletText("Zero injecao de DLL ou modificacao de arquivos do Discord");
  ImGui::BulletText("Zero aumento de ping em conexoes de jogos e downloads");
  ImGui::BulletText("Compativel com Discord Oficial, Canary, PTB e Vesktop");
  ImGui::BulletText("Interface Grafica Dear ImGui com Renderizacao Estatica OpenGL");

  ImGui::Spacing();
  ImGui::Separator();
  ImGui::Spacing();

  if (ImGui::Button("[ REPOSITORIO GITHUB ]", ImVec2(220.0f, 36.0f))) {
    openBrowser("https://github.com/alrigroup/ARDiscordBypass");
  }
  ImGui::SameLine(0.0f, 10.0f);
  if (ImGui::Button("[ NOTAS DE FUNCIONAMENTO ]", ImVec2(240.0f, 36.0f))) {
    m_showNotesModal = true;
  }
  ImGui::SameLine(0.0f, 10.0f);
  if (ImGui::Button("[ CREDITOS & EQUIPE ]", ImVec2(200.0f, 36.0f))) {
    m_showCreditsModal = true;
  }

  ImGui::EndChild();
}

void AppWindow::renderNotesModal() {
  if (!m_showNotesModal) return;

  const ThemeColors &th = getTheme();
  ImGui::OpenPopup("##NotesModal");

  ImVec2 displaySize = ImGui::GetIO().DisplaySize;
  ImVec2 modalSize = ImVec2(580.0f, 460.0f);
  if (modalSize.x > displaySize.x - 20.0f) modalSize.x = displaySize.x - 20.0f;
  if (modalSize.y > displaySize.y - 20.0f) modalSize.y = displaySize.y - 20.0f;

  ImGui::SetNextWindowSize(modalSize, ImGuiCond_Always);
  ImGui::SetNextWindowPos(ImVec2(displaySize.x * 0.5f, displaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));

  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.5f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16.0f, 16.0f));
  ImGui::PushStyleColor(ImGuiCol_PopupBg, th.card);
  ImGui::PushStyleColor(ImGuiCol_Border, th.alriRed);

  if (ImGui::BeginPopupModal("##NotesModal", &m_showNotesModal, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar)) {
    ImGui::TextColored(th.alriRed, "NOTAS DE FUNCIONAMENTO");
    ImGui::TextColored(th.textMuted, "Entenda a arquitetura de isolamento e liberacao do Discord");
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::BeginChild("NotesScroll", ImVec2(0, -48.0f), true);

    ImGui::TextColored(th.terminalYellow, "COMO FUNCIONA O BYPASS:");
    ImGui::Spacing();

    ImGui::TextColored(th.terminalGreen, "1. Roteamento por Processo (Split-Tunneling):");
    ImGui::TextWrapped("O ARDiscordBypass utiliza a tecnologia WireGuard e tecnicas de baixo nivel (Network Namespaces no Linux e Windows Filtering Platform via WireSock no Windows) para criar um tunel exclusivo apenas para os executaveis do Discord.");
    ImGui::Spacing();

    ImGui::TextColored(th.terminalGreen, "2. Sua Internet Fica Intocada:");
    ImGui::TextWrapped("Todo o restante do computador (Steam, jogos online, navegadores, downloads, bancos e WhatsApp) permanece 100%% na sua conexao normal, sem consumir o tunel e mantendo latencia nativa.");
    ImGui::Spacing();

    ImGui::TextColored(th.terminalGreen, "3. Liberacao de Transmissao e Camera:");
    ImGui::TextWrapped("Ao entrar em salas de voz ou transmitir tela, os pacotes WebRTC trafegam por uma rota internacional de baixa latencia, contornando bloqueios de operadoras sem afetar o restante da sua maquina.");
    ImGui::Spacing();

    ImGui::TextColored(th.terminalGreen, "4. Seguranca & Antiban:");
    ImGui::TextWrapped("Zero modificacao de binarios do Discord, zero injecao de DLLs ou memoria. 100%% em conformidade com as diretrizes do sistema operacional.");

    ImGui::EndChild();

    ImGui::Spacing();
    if (ImGui::Button("  [ ENTENDIDO ]  ", ImVec2(-1, 36.0f))) {
      m_showNotesModal = false;
      ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
  }

  ImGui::PopStyleColor(2);
  ImGui::PopStyleVar(3);
}

void AppWindow::renderCreditsModal() {
  if (!m_showCreditsModal) return;

  const ThemeColors &th = getTheme();
  ImGui::OpenPopup("##CreditsModal");

  ImVec2 displaySize = ImGui::GetIO().DisplaySize;
  ImVec2 modalSize = ImVec2(540.0f, 420.0f);
  if (modalSize.x > displaySize.x - 20.0f) modalSize.x = displaySize.x - 20.0f;
  if (modalSize.y > displaySize.y - 20.0f) modalSize.y = displaySize.y - 20.0f;

  ImGui::SetNextWindowSize(modalSize, ImGuiCond_Always);
  ImGui::SetNextWindowPos(ImVec2(displaySize.x * 0.5f, displaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));

  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.5f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16.0f, 16.0f));
  ImGui::PushStyleColor(ImGuiCol_PopupBg, th.card);
  ImGui::PushStyleColor(ImGuiCol_Border, th.alriRed);

  if (ImGui::BeginPopupModal("##CreditsModal", &m_showCreditsModal, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar)) {
    ImGui::TextColored(th.alriRed, "CREDITOS & AUTOR");
    ImGui::TextColored(th.textMuted, "ARDiscordBypass v3.5.0 — ALRI GROUP");
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::BeginChild("CreditsScroll", ImVec2(0, -48.0f), true);

    ImGui::TextColored(th.textPrimary, "Desenvolvimento & Autor:");
    ImGui::TextColored(th.alriRed, "Alexsanderalri");
    ImGui::Spacing();

    ImGui::TextColored(th.textPrimary, "Organizacao:");
    ImGui::TextColored(th.textSecondary, "ALRI Group (Copyright (c) 2020-2026)");
    ImGui::Spacing();

    ImGui::TextColored(th.textPrimary, "Licenca de Software:");
    ImGui::TextColored(th.textMuted, "ARGLFU (ALRI GROUP LICENSE FREE USE) Version 2");
    ImGui::Spacing();

    ImGui::TextColored(th.textPrimary, "Repositorio Oficial:");
    ImGui::TextColored(ImVec4(0.0f, 0.82f, 1.0f, 1.0f), "https://github.com/alrigroup/ARDiscordBypass");
    ImGui::Spacing();

    ImGui::TextColored(th.textPrimary, "Protecao de Conexao:");
    ImGui::TextColored(th.terminalGreen, "WireGuard Process-Isolated Tunnel Engine");

    ImGui::EndChild();

    ImGui::Spacing();
    if (ImGui::Button("  [ FECHAR ]  ", ImVec2(-1, 36.0f))) {
      m_showCreditsModal = false;
      ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
  }

  ImGui::PopStyleColor(2);
  ImGui::PopStyleVar(3);
}

void AppWindow::renderLoadingOverlay() {
  const ThemeColors &th = getTheme();
  ImVec2 displaySize = ImGui::GetIO().DisplaySize;

  ImGui::SetNextWindowPos(ImVec2(0, 0));
  ImGui::SetNextWindowSize(displaySize);
  ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0.85f));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

  if (ImGui::Begin("##AuthLoadingOverlay", nullptr,
                   ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                   ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus)) {

    ImDrawList *drawList = ImGui::GetWindowDrawList();

    // 1. Fundo escuro semi-transparente (#D9000000 = 85% alpha)
    drawList->AddRectFilled(ImVec2(0, 0), displaySize, IM_COL32(0, 0, 0, 218));

    // 2. Card Centralizado estilo Box Terminal Android
    float cardW = 440.0f;
    if (cardW > displaySize.x - 32.0f) cardW = displaySize.x - 32.0f;
    float cardH = (!m_authError.empty()) ? 300.0f : 250.0f;

    ImVec2 center = ImVec2(displaySize.x * 0.5f, displaySize.y * 0.5f);
    ImVec2 cardMin = ImVec2(center.x - cardW * 0.5f, center.y - cardH * 0.5f);
    ImVec2 cardMax = ImVec2(center.x + cardW * 0.5f, center.y + cardH * 0.5f);

    drawList->AddRectFilled(cardMin, cardMax, ImGui::GetColorU32(th.card), 12.0f);
    drawList->AddRect(cardMin, cardMax, ImGui::GetColorU32(th.cardBorder), 12.0f, 0, 1.5f);

    if (m_isAuthWorking || !m_authError.empty() || m_authSuccess) {
      // 1. Logo ALRI Animada (Pulsing Scale 1.0 -> 1.15 e Alpha 0.6 -> 1.0 com 900ms cycle, igual ao APK!)
      float t = static_cast<float>(ImGui::GetTime());
      float pulsePhase = (std::sin(t * (2.0f * 3.14159265f / 0.9f)) + 1.0f) * 0.5f;
      float scale = (m_authSuccess || !m_authError.empty()) ? 1.0f : (1.0f + 0.15f * pulsePhase);
      int alpha = (m_authSuccess || !m_authError.empty()) ? 255 : static_cast<int>((0.6f + 0.4f * pulsePhase) * 255.0f);

      float baseLogoSize = 54.0f;
      float currentLogoSize = baseLogoSize * scale;
      ImVec2 logoCenter = ImVec2(center.x, cardMin.y + 44.0f);
      ImVec2 logoMin = ImVec2(logoCenter.x - currentLogoSize * 0.5f, logoCenter.y - currentLogoSize * 0.5f);
      ImVec2 logoMax = ImVec2(logoCenter.x + currentLogoSize * 0.5f, logoCenter.y + currentLogoSize * 0.5f);

      if (m_logoTexId != 0) {
        drawList->AddImage(static_cast<ImTextureID>(m_logoTexId), logoMin, logoMax,
                           ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 255, 255, alpha));
      }

      // 2. Spinner Circular alri_red (pbAuthLoading)
      ImVec2 spinnerCenter = ImVec2(center.x, cardMin.y + 98.0f);
      if (m_authSuccess) {
        drawList->AddCircleFilled(spinnerCenter, 13.0f, ImGui::GetColorU32(th.terminalGreen), 20);
        drawList->AddText(ImVec2(spinnerCenter.x - 5.0f, spinnerCenter.y - 7.0f), IM_COL32_BLACK, "v");
      } else if (!m_authError.empty()) {
        drawList->AddCircleFilled(spinnerCenter, 13.0f, ImGui::GetColorU32(th.alriRed), 20);
        drawList->AddText(ImVec2(spinnerCenter.x - 4.0f, spinnerCenter.y - 7.0f), IM_COL32_WHITE, "x");
      } else {
        DrawRotatingSpinner(drawList, spinnerCenter, 15.0f, 3.2f, ImGui::GetColorU32(th.alriRed));
      }

      // 3. Título Monospace
      std::string titleStr;
      ImVec4 titleCol;
      if (m_authSuccess) {
        titleStr = "[ ✔ ] AUTENTICADO COM SUCESSO!";
        titleCol = th.terminalGreen;
      } else if (!m_authError.empty()) {
        titleStr = "[ ✕ ] FALHA NA AUTENTICACAO";
        titleCol = th.alriRed;
      } else {
        titleStr = "AUTENTICANDO PROTONVPN";
        titleCol = th.alriRed;
      }

      ImVec2 titleSz = ImGui::CalcTextSize(titleStr.c_str());
      ImVec2 titlePos = ImVec2(center.x - titleSz.x * 0.5f, cardMin.y + 126.0f);
      drawList->AddText(titlePos, ImGui::GetColorU32(titleCol), titleStr.c_str());

      // 4. Status / Subtítulo
      std::string statusStr;
      ImVec4 statusCol = th.textSecondary;
      if (m_authSuccess) {
        statusStr = "Túnel WireGuard configurado. Inicializando bypass...";
        statusCol = th.terminalGreen;
      } else if (!m_authError.empty()) {
        statusStr = m_authError;
        statusCol = th.terminalYellow;
      } else {
        int dotCount = static_cast<int>(ImGui::GetTime() * 2.5) % 4;
        std::string dots = (dotCount == 1) ? "." : (dotCount == 2) ? ".." : (dotCount == 3) ? "..." : "";
        statusStr = m_workMessage;
        if (!statusStr.empty() && statusStr.back() != '.') {
          statusStr += dots;
        }
      }

      ImVec2 statusSz = ImGui::CalcTextSize(statusStr.c_str());
      if (statusSz.x > cardW - 36.0f) statusSz.x = cardW - 36.0f;
      ImVec2 statusPos = ImVec2(center.x - statusSz.x * 0.5f, cardMin.y + 152.0f);
      drawList->AddText(statusPos, ImGui::GetColorU32(statusCol), statusStr.c_str());

      // 5. Botão de Retorno (se erro) ou Barra de Progresso (se carregando)
      if (!m_authError.empty()) {
        float btnW = cardW - 48.0f;
        float btnH = 38.0f;
        ImGui::SetCursorScreenPos(ImVec2(center.x - btnW * 0.5f, cardMax.y - 54.0f));

        ImGui::PushStyleColor(ImGuiCol_Button, th.surface);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, th.cardBorder);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, th.card);
        ImGui::PushStyleColor(ImGuiCol_Text, th.textPrimary);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);

        if (ImGui::Button("[ VOLTAR E CORRIGIR ]", ImVec2(btnW, btnH))) {
          m_isAuthWorking = false;
          m_isWorking = false;
          m_authError.clear();
          m_showSetupModal = true;
        }

        ImGui::PopStyleVar();
        ImGui::PopStyleColor(4);
      } else {
        ImVec2 barMin = ImVec2(cardMin.x + 36.0f, cardMax.y - 28.0f);
        ImVec2 barMax = ImVec2(cardMax.x - 36.0f, cardMax.y - 22.0f);
        DrawIndeterminateProgressBar(drawList, barMin, barMax, ImGui::GetColorU32(th.surface),
                                    m_authSuccess ? ImGui::GetColorU32(th.terminalGreen) : ImGui::GetColorU32(th.alriRed));
      }
    } else {
      // Operação genérica em segundo plano
      ImVec2 spinnerCenter = ImVec2(center.x, cardMin.y + 50.0f);
      DrawRotatingSpinner(drawList, spinnerCenter, 18.0f, 3.5f, ImGui::GetColorU32(th.alriRed));

      int dotCount = static_cast<int>(ImGui::GetTime() * 2.5) % 4;
      std::string dots = (dotCount == 1) ? "." : (dotCount == 2) ? ".." : (dotCount == 3) ? "..." : "";
      std::string headerMsg = "Operacao em andamento" + dots;

      ImVec2 textSz = ImGui::CalcTextSize(headerMsg.c_str());
      ImVec2 textPos = ImVec2(center.x - textSz.x * 0.5f, cardMin.y + 85.0f);
      drawList->AddText(textPos, ImGui::GetColorU32(th.terminalYellow), headerMsg.c_str());

      ImVec2 subSz = ImGui::CalcTextSize(m_workMessage.c_str());
      if (subSz.x > cardW - 32.0f) subSz.x = cardW - 32.0f;
      ImVec2 subPos = ImVec2(center.x - subSz.x * 0.5f, cardMin.y + 115.0f);
      drawList->AddText(subPos, ImGui::GetColorU32(th.textSecondary), m_workMessage.c_str());

      ImVec2 barMin = ImVec2(cardMin.x + 30.0f, cardMax.y - 28.0f);
      ImVec2 barMax = ImVec2(cardMax.x - 30.0f, cardMax.y - 20.0f);
      DrawIndeterminateProgressBar(drawList, barMin, barMax, ImGui::GetColorU32(th.surface), ImGui::GetColorU32(th.alriRed));
    }
  }
  ImGui::End();
  ImGui::PopStyleVar(2);
  ImGui::PopStyleColor();
}

void AppWindow::renderSplashOverlay() {
  if (m_splashDone) return;

  float elapsed = static_cast<float>(ImGui::GetTime()) - m_splashStartTime;
  if (elapsed >= 1.8f) {
    m_splashDone = true;
    if (!m_modalAutoTriggered && !hasValidConfig()) {
      m_showSetupModal = true;
      m_modalAutoTriggered = true;
    }
    return;
  }

  const ThemeColors &th = getTheme();
  ImDrawList *drawList = ImGui::GetForegroundDrawList();
  ImVec2 displaySize = ImGui::GetIO().DisplaySize;

  ImVec2 center = ImVec2(displaySize.x * 0.5f, displaySize.y * 0.5f - 20.0f);
  ImVec2 targetHeaderLogo = ImVec2(16.0f + 21.0f, 16.0f + 21.0f);

  if (elapsed < 1.1f) {
    // Fase 1: Rotação suave da logo ALRI, respiração e anel giratório (Estilo Splash Android)
    drawList->AddRectFilled(ImVec2(0, 0), displaySize, IM_COL32(0, 0, 0, 255));

    float angle = (elapsed / 1.1f) * (2.0f * 3.14159265f);
    float pulse = 1.0f + 0.12f * std::sin(elapsed * 4.0f);
    float logoSize = 100.0f * pulse;
    ImVec2 hs = ImVec2(logoSize * 0.5f, logoSize * 0.5f);

    // Anel orbital de carregamento
    DrawRotatingSpinner(drawList, center, 72.0f, 3.5f, ImGui::GetColorU32(th.alriRed));

    // Logo rotativa com imagem quad
    if (m_logoTexId != 0) {
      float cos_a = std::cos(angle);
      float sin_a = std::sin(angle);
      ImVec2 p0 = ImVec2(center.x + (-hs.x * cos_a - -hs.y * sin_a), center.y + (-hs.x * sin_a + -hs.y * cos_a));
      ImVec2 p1 = ImVec2(center.x + ( hs.x * cos_a - -hs.y * sin_a), center.y + ( hs.x * sin_a + -hs.y * cos_a));
      ImVec2 p2 = ImVec2(center.x + ( hs.x * cos_a -  hs.y * sin_a), center.y + ( hs.x * sin_a +  hs.y * cos_a));
      ImVec2 p3 = ImVec2(center.x + (-hs.x * cos_a -  hs.y * sin_a), center.y + (-hs.x * sin_a +  hs.y * cos_a));
      drawList->AddImageQuad(static_cast<ImTextureID>(m_logoTexId), p0, p1, p2, p3, ImVec2(0, 0), ImVec2(1, 0), ImVec2(1, 1), ImVec2(0, 1), IM_COL32_WHITE);
    }

    const char *title = "ARDiscordBypass v3.5.0";
    ImVec2 titleSz = ImGui::CalcTextSize(title);
    drawList->AddText(ImVec2(center.x - titleSz.x * 0.5f, center.y + 85.0f), ImGui::GetColorU32(th.alriRed), title);

    const char *sub = "ALRI GROUP // INICIALIZANDO...";
    ImVec2 subSz = ImGui::CalcTextSize(sub);
    drawList->AddText(ImVec2(center.x - subSz.x * 0.5f, center.y + 110.0f), ImGui::GetColorU32(th.textSecondary), sub);
  } else {
    // Fase 2: Transição de UNLOCK (1.1s a 1.8s) - A logo sobe para o cabeçalho e o app é "puxado"
    float t = (elapsed - 1.1f) / 0.7f;
    if (t > 1.0f) t = 1.0f;
    float ease = 1.0f - std::pow(1.0f - t, 3.0f);

    int bgAlpha = static_cast<int>((1.0f - ease) * 255.0f);
    if (bgAlpha > 0) {
      drawList->AddRectFilled(ImVec2(0, 0), displaySize, IM_COL32(0, 0, 0, bgAlpha));
    }

    ImVec2 curCenter = ImVec2(
      center.x + (targetHeaderLogo.x - center.x) * ease,
      center.y + (targetHeaderLogo.y - center.y) * ease
    );
    float curSize = 100.0f + (42.0f - 100.0f) * ease;
    ImVec2 hs = ImVec2(curSize * 0.5f, curSize * 0.5f);

    if (m_logoTexId != 0) {
      ImVec2 p0 = ImVec2(curCenter.x - hs.x, curCenter.y - hs.y);
      ImVec2 p1 = ImVec2(curCenter.x + hs.x, curCenter.y - hs.y);
      ImVec2 p2 = ImVec2(curCenter.x + hs.x, curCenter.y + hs.y);
      ImVec2 p3 = ImVec2(curCenter.x - hs.x, curCenter.y + hs.y);
      drawList->AddImageQuad(static_cast<ImTextureID>(m_logoTexId), p0, p1, p2, p3, ImVec2(0, 0), ImVec2(1, 0), ImVec2(1, 1), ImVec2(0, 1), IM_COL32_WHITE);
    }
  }
}

bool AppWindow::hasValidConfig() const {
  if (m_configMode == 1) {
    return (m_customPathBuf[0] != '\0' && fs::is_regular_file(m_customPathBuf));
  }
  return (fs::is_regular_file("ardc_vpn.conf") || Core::Service::instance().hasSavedSession());
}

void AppWindow::openBrowser(const std::string &url) {
#ifdef _WIN32
  ShellExecuteA(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
#else
  std::string cmd = "xdg-open '" + url + "' >/dev/null 2>&1 &";
  std::system(cmd.c_str());
#endif
}

void AppWindow::renderSetupModal() {
  if (!m_showSetupModal) return;

  const ThemeColors &th = getTheme();
  ImGui::OpenPopup("##SetupConfigModal");

  ImVec2 displaySize = ImGui::GetIO().DisplaySize;
  ImVec2 modalSize = ImVec2(660.0f, 570.0f);
  if (modalSize.x > displaySize.x - 20.0f) modalSize.x = displaySize.x - 20.0f;
  if (modalSize.y > displaySize.y - 20.0f) modalSize.y = displaySize.y - 20.0f;

  ImGui::SetNextWindowSize(modalSize, ImGuiCond_Always);
  ImGui::SetNextWindowPos(ImVec2(displaySize.x * 0.5f, displaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));

  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.5f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16.0f, 16.0f));
  ImGui::PushStyleColor(ImGuiCol_PopupBg, th.card);
  ImGui::PushStyleColor(ImGuiCol_Border, th.alriRed);

  if (ImGui::BeginPopupModal("##SetupConfigModal", &m_showSetupModal, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar)) {
    ImGui::BeginChild("SetupModalScroll", ImVec2(0, 0), false);

    // Cabeçalho Modal
    ImGui::BeginGroup();
    if (m_logoTexId != 0) {
      ImGui::Image(static_cast<ImTextureID>(m_logoTexId), ImVec2(32.0f, 32.0f));
      ImGui::SameLine(0.0f, 12.0f);
    }
    ImGui::BeginGroup();
    ImGui::TextColored(th.alriRed, "CONFIGURACAO WIREGUARD NECESSARIA");
    ImGui::TextColored(th.textMuted, "Selecione o metodo de conexao para ativar o bypass");
    ImGui::EndGroup();
    ImGui::EndGroup();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // ==========================================
    // OPÇÃO 1: PROTONVPN GRATUITA (RECOMENDADO)
    // ==========================================
    ImGui::PushStyleColor(ImGuiCol_ChildBg, th.surface);
    ImGui::BeginChild("ProtonChoiceCard", ImVec2(0, 240.0f), true, ImGuiWindowFlags_None);

    ImGui::TextColored(th.textPrimary, "CONTA PROTONVPN GRATUITA");
    ImGui::SameLine();
    ImGui::TextColored(th.terminalGreen, "[ RECOMENDADO ]");

    ImGui::TextColored(th.textMuted, "Acesso internacional 100%% gratuito e ilimitado para chamadas e streamings no Discord sem custo.");
    ImGui::Spacing();

    // Inputs de credenciais
    ImGui::TextColored(th.textSecondary, "Usuario Proton:");
    ImGui::PushItemWidth(-1);
    ImGui::InputText("##modalProtonUser", m_protonUserBuf, sizeof(m_protonUserBuf));
    ImGui::PopItemWidth();

    ImGui::TextColored(th.textSecondary, "Senha Proton:");
    ImGui::PushItemWidth(ImGui::GetWindowWidth() - 95.0f);
    ImGuiInputTextFlags pFlags = m_hidePassword ? ImGuiInputTextFlags_Password : 0;
    ImGui::InputText("##modalProtonPass", m_protonPassBuf, sizeof(m_protonPassBuf), pFlags);
    ImGui::PopItemWidth();
    ImGui::SameLine();
    if (ImGui::Button(m_hidePassword ? "Mostrar" : "Ocultar", ImVec2(75.0f, 0.0f))) {
      m_hidePassword = !m_hidePassword;
    }

    ImGui::Spacing();

    // Botões da Opção 1: Entrar & Ativar + Criar Conta
    float availW = ImGui::GetContentRegionAvail().x;
    float btnW = (availW - 10.0f) * 0.5f;

    ImGui::PushStyleColor(ImGuiCol_Button, th.alriRed);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, th.alriRedGlow);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, th.alriRedDark);
    ImGui::PushStyleColor(ImGuiCol_Text, th.btnText);

    if (ImGui::Button("1. ENTRAR & ATIVAR BYPASS", ImVec2(btnW, 36.0f)) && !m_isWorking) {
      m_showSetupModal = false;
      ImGui::CloseCurrentPopup();
      triggerProtonLoginAndStart();
    }
    ImGui::PopStyleColor(4);

    ImGui::SameLine(0.0f, 10.0f);
    if (ImGui::Button("Criar Conta / Esqueceu Senha (Web)", ImVec2(btnW, 36.0f))) {
      openBrowser("https://account.protonvpn.com/pt-br/login");
      Core::Service::instance().addLog("Abrindo pagina oficial de login ProtonVPN no navegador...", 0);
    }

    ImGui::EndChild();
    ImGui::PopStyleColor(); // ChildBg

    ImGui::Spacing();

    // ==========================================
    // OPÇÃO 2: PERFIL CUSTOMIZADO (.CONF)
    // ==========================================
    ImGui::PushStyleColor(ImGuiCol_ChildBg, th.surface);
    ImGui::BeginChild("CustomChoiceCard", ImVec2(0, 135.0f), true, ImGuiWindowFlags_None);

    ImGui::TextColored(th.textPrimary, "PERFIL CUSTOMIZADO (.CONF)");
    ImGui::TextColored(th.textMuted, "Utilize seu proprio arquivo WireGuard (.conf) de VPS propria, WARP ou Mullvad.");
    ImGui::Spacing();

    ImGui::TextColored(th.textSecondary, "Caminho do arquivo:");
    ImGui::PushItemWidth(ImGui::GetWindowWidth() - 130.0f);
    ImGui::InputText("##modalCustomPath", m_customPathBuf, sizeof(m_customPathBuf));
    ImGui::PopItemWidth();
    ImGui::SameLine();
    if (ImGui::Button(" Procurar... ", ImVec2(100.0f, 0.0f)) && !m_isWorking) {
      pickConfigFile();
    }

    ImGui::Spacing();
    if (ImGui::Button("2. USAR ARQUIVO .CONF & ATIVAR BYPASS", ImVec2(-1, 32.0f)) && !m_isWorking) {
      if (m_customPathBuf[0] == '\0' || !fs::is_regular_file(m_customPathBuf)) {
        Core::Service::instance().addLog("[AVISO] Selecione um arquivo .conf valido antes de prosseguir.", 2);
      } else {
        m_configMode = 1;
        m_showSetupModal = false;
        ImGui::CloseCurrentPopup();
        triggerStart();
      }
    }

    ImGui::EndChild();
    ImGui::PopStyleColor(); // ChildBg

    ImGui::Spacing();

    // Botão de Fechar / Cancelar
    if (ImGui::Button(" Fechar ", ImVec2(110.0f, 32.0f))) {
      m_showSetupModal = false;
      ImGui::CloseCurrentPopup();
    }

    ImGui::EndChild(); // Scroll
    ImGui::EndPopup();
  }

  ImGui::PopStyleColor(2);
  ImGui::PopStyleVar(3);
}

void AppWindow::triggerStart() {
  if (m_isWorking) return;

  if (!hasValidConfig()) {
    Core::Service::instance().addLog("[AVISO] Nenhuma configuracao salva. Escolha Proton Gratis (Recomendado) ou Importe .conf.", 2);
    m_showSetupModal = true;
    return;
  }

  m_isWorking = true;
  m_isConnecting = true;
  m_isStopping = false;
  m_workMessage = "Inicializando isolamento de rede...";

  std::string chosenConf = (m_configMode == 1 && m_customPathBuf[0] != '\0') ? m_customPathBuf : "ardc_vpn.conf";

  m_future = std::async(std::launch::async, [this, chosenConf]() {
    Core::Service::instance().addLog("[ ⠋ ] Preparando túnel WireGuard dedicado e isolamento de rede...", 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    Core::Service::instance().addLog("[ ⠙ ] Estabelecendo handshake com servidor internacional...", 0);

    std::string err;
    bool ok = Core::Service::instance().startBypass(chosenConf, err, [this](const std::string &msg) {
      m_workMessage = msg;
    });
    if (!ok && !err.empty()) {
      Core::Service::instance().addLog(err, 3);
    } else if (ok) {
      Core::Service::instance().addLog("[ ✔ ] Túnel conectado! Discord liberado para Transmissão e Câmera.", 1);
    }
    m_status = Core::Service::instance().queryStatus(chosenConf);
    m_isConnecting = false;
    m_isWorking = false;
  });
}

void AppWindow::triggerStop() {
  if (m_isWorking) return;
  m_isWorking = true;
  m_isConnecting = true;
  m_isStopping = true;
  m_workMessage = "Restaurando conexao e removendo namespace...";

  m_future = std::async(std::launch::async, [this]() {
    Core::Service::instance().addLog("[ ⠋ ] Encerrando túnel WireGuard isolado...", 0);
    std::string err;
    Core::Service::instance().stopAll(err);
    Core::Service::instance().addLog("[ ✔ ] Conexão restaurada. Tráfego normalizado.", 1);
    m_status = Core::Service::instance().queryStatus();
    m_isStopping = false;
    m_isConnecting = false;
    m_isWorking = false;
  });
}

void AppWindow::triggerProtonLogin() {
  if (m_isWorking) return;
  std::string user = m_protonUserBuf;
  std::string pass = m_protonPassBuf;
  if (user.empty() || pass.empty()) {
    Core::Service::instance().addLog("Usuario e senha sao obrigatorios para autenticacao.", 2);
    return;
  }

  m_isWorking = true;
  m_isAuthWorking = true;
  m_authSuccess = false;
  m_authError.clear();
  m_authStartTime = static_cast<float>(ImGui::GetTime());
  m_workMessage = "Iniciando handshake seguro via SRP na API Proton...";

  m_future = std::async(std::launch::async, [this, user, pass]() {
    Core::Service::instance().addLog("[ ⠋ ] Autenticando via SRP na API ProtonVPN...", 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    m_workMessage = "Validando credenciais e gerando chaves WireGuard...";

    std::string err;
    bool ok = Core::Service::instance().loginProton(user, pass, err, [this](const std::string &stage) {
      m_workMessage = stage;
      Core::Service::instance().addLog("[ ⠋ ] " + stage, 0);
    });
    if (!ok) {
      m_authError = !err.empty() ? err : "Falha na autenticação ou timeout da API Proton.";
      Core::Service::instance().addLog(m_authError, 3);
      m_status = Core::Service::instance().queryStatus("ardc_vpn.conf");
      return;
    }

    m_configMode = 0;
    m_authSuccess = true;
    m_workMessage = "Perfil WireGuard ProtonVPN gerado com sucesso!";
    Core::Service::instance().addLog("[ ✔ ] Perfil ProtonVPN configurado com sucesso!", 1);
    std::this_thread::sleep_for(std::chrono::milliseconds(800));

    m_status = Core::Service::instance().queryStatus("ardc_vpn.conf");
    m_authSuccess = false;
    m_isAuthWorking = false;
    m_isWorking = false;
  });
}

void AppWindow::triggerProtonLoginAndStart() {
  if (m_isWorking) return;
  std::string user = m_protonUserBuf;
  std::string pass = m_protonPassBuf;
  if (user.empty() || pass.empty()) {
    Core::Service::instance().addLog("[AVISO] Usuario e senha sao obrigatorios para autenticacao Proton.", 2);
    m_showSetupModal = true;
    return;
  }

  m_isWorking = true;
  m_isAuthWorking = true;
  m_authSuccess = false;
  m_authError.clear();
  m_authStartTime = static_cast<float>(ImGui::GetTime());
  m_workMessage = "Iniciando handshake seguro via SRP na API Proton...";

  m_future = std::async(std::launch::async, [this, user, pass]() {
    Core::Service::instance().addLog("[ ⠋ ] Autenticando via SRP na API ProtonVPN...", 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    m_workMessage = "Validando credenciais e obtendo servidores livres...";

    std::string err;
    bool ok = Core::Service::instance().loginProton(user, pass, err, [this](const std::string &stage) {
      m_workMessage = stage;
      Core::Service::instance().addLog("[ ⠋ ] " + stage, 0);
    });
    if (!ok) {
      m_authError = !err.empty() ? err : "Falha na autenticação ou timeout da API Proton.";
      Core::Service::instance().addLog(m_authError, 3);
      m_status = Core::Service::instance().queryStatus("ardc_vpn.conf");
      return;
    }

    m_configMode = 0;
    m_authSuccess = true;
    m_workMessage = "Túnel WireGuard configurado. Inicializando bypass...";
    Core::Service::instance().addLog("[ ✔ ] Autenticado com sucesso! Inicializando bypass...", 1);
    std::this_thread::sleep_for(std::chrono::milliseconds(800));

    // Transição suave para conexão (exatamente como o APK!)
    m_authSuccess = false;
    m_isAuthWorking = false;
    m_isConnecting = true;
    m_isStopping = false;
    m_workMessage = "Ativando túnel WireGuard isolado...";

    Core::Service::instance().addLog("[ ⠋ ] Preparando túnel WireGuard dedicado e isolamento de rede...", 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    Core::Service::instance().addLog("[ ⠙ ] Estabelecendo handshake com servidor internacional...", 0);

    bool startOk = Core::Service::instance().startBypass("ardc_vpn.conf", err, [this](const std::string &msg) {
      m_workMessage = msg;
    });
    if (!startOk && !err.empty()) {
      Core::Service::instance().addLog(err, 3);
    } else if (startOk) {
      Core::Service::instance().addLog("[ ✔ ] Túnel conectado! Discord liberado para Transmissão e Câmera.", 1);
    }

    m_status = Core::Service::instance().queryStatus("ardc_vpn.conf");
    m_isConnecting = false;
    m_isWorking = false;
  });
}

void AppWindow::triggerClearAll() {
  if (m_isWorking) return;
  m_isWorking = true;
  m_workMessage = "Excluindo arquivos de configuracao e sessao...";

  m_future = std::async(std::launch::async, [this]() {
    std::string err;
    Core::Service::instance().clearAllConfigAndSession(err);
    m_customPathBuf[0] = '\0';
    m_protonUserBuf[0] = '\0';
    m_protonPassBuf[0] = '\0';
    m_status = Core::Service::instance().queryStatus();
    m_isWorking = false;
  });
}

void AppWindow::pickConfigFile() {
  std::string picked = openNativeFileDialog();
  if (!picked.empty()) {
    std::strncpy(m_customPathBuf, picked.c_str(), sizeof(m_customPathBuf) - 1);
    m_customPathBuf[sizeof(m_customPathBuf) - 1] = '\0';
    Core::Service::instance().addLog("Arquivo .conf selecionado: " + picked, 0);
  }
}

} // namespace GUI
