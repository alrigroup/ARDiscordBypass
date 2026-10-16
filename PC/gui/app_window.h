#pragma once

#include <string>
#include <atomic>
#include <future>
#include <vector>
#include <chrono>
#include "../core/service.h"

namespace GUI {

class AppWindow {
public:
  AppWindow();
  ~AppWindow();

  void setLogoTexture(unsigned int texId) { m_logoTexId = texId; }
  void setDarkMode(bool dark);
  bool isDarkMode() const { return m_darkMode; }

  void render();
  bool isAnimating() const {
    return m_isWorking || m_isConnecting || m_isAuthWorking || !m_splashDone;
  }

private:
  void renderHeader();
  void renderNavigation();
  void renderDashboard();
  void renderConfigTab();
  void renderLogsTab();
  void renderAboutTab();
  void renderLoadingOverlay();
  void renderSplashOverlay();
  void renderSetupModal();
  void renderNotesModal();
  void renderCreditsModal();

  void triggerStart();
  void triggerStop();
  void triggerProtonLogin();
  void triggerProtonLoginAndStart();
  void triggerClearAll();
  void pickConfigFile();
  void openBrowser(const std::string &url);

  unsigned int m_logoTexId = 0;
  bool m_darkMode = true;

  // Estado da Splash / Transição Unlock
  bool m_splashDone = false;
  float m_splashStartTime = 0.0f;

  // Animação de Transição de Abas
  float m_tabAnimStartTime = 0.0f;

  // Modal de Configuração Obrigatória e Diálogos Sobre
  bool m_showSetupModal = false;
  bool m_showNotesModal = false;
  bool m_showCreditsModal = false;
  bool m_modalAutoTriggered = false;
  bool hasValidConfig() const;

  // Estado da UI
  int m_configMode = 0; // 0 = Proton, 1 = Custom .conf
  char m_customPathBuf[512] = "";
  char m_protonUserBuf[128] = "";
  char m_protonPassBuf[128] = "";
  bool m_hidePassword = true;

  std::atomic<bool> m_isWorking{false};
  std::atomic<bool> m_isConnecting{false};
  bool m_isStopping = false;
  std::atomic<bool> m_isAuthWorking{false};
  bool m_authSuccess = false;
  float m_authSuccessTime = 0.0f;
  std::string m_authError;
  float m_authStartTime = 0.0f;
  std::string m_workMessage;
  std::future<void> m_future;

  Core::TunnelStatus m_status;
  std::chrono::steady_clock::time_point m_lastProbe;

  int m_activeTab = 0; // 0 = Dashboard, 1 = Configurações, 2 = Logs, 3 = Sobre
};

} // namespace GUI
