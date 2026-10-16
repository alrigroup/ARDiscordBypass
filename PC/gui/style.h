#pragma once

#include "vendor/imgui/imgui.h"

namespace GUI {

struct ThemeColors {
  bool isDark;
  ImVec4 bg;
  ImVec4 surface;
  ImVec4 card;
  ImVec4 cardBorder;
  ImVec4 alriRed;
  ImVec4 alriRedDark;
  ImVec4 alriRedGlow;
  ImVec4 terminalGreen;
  ImVec4 terminalYellow;
  ImVec4 textPrimary;
  ImVec4 textSecondary;
  ImVec4 textMuted;
  ImVec4 btnText;
};

inline ThemeColors g_currentTheme;

inline const ThemeColors &getTheme() {
  return g_currentTheme;
}

inline void applyTheme(bool darkMode) {
  g_currentTheme.isDark = darkMode;

  if (darkMode) {
    // 100% OLED Pure Black (Android values-night/colors.xml)
    g_currentTheme.bg             = ImVec4(0.000f, 0.000f, 0.000f, 1.00f); // #000000
    g_currentTheme.surface        = ImVec4(0.039f, 0.039f, 0.039f, 1.00f); // #0A0A0A
    g_currentTheme.card           = ImVec4(0.071f, 0.071f, 0.071f, 1.00f); // #121212
    g_currentTheme.cardBorder     = ImVec4(0.141f, 0.141f, 0.141f, 1.00f); // #242424
    g_currentTheme.alriRed        = ImVec4(0.898f, 0.035f, 0.078f, 1.00f); // #E50914
    g_currentTheme.alriRedDark    = ImVec4(0.698f, 0.027f, 0.063f, 1.00f); // #B20710
    g_currentTheme.alriRedGlow    = ImVec4(1.000f, 0.118f, 0.153f, 1.00f); // #FF1E27
    g_currentTheme.terminalGreen  = ImVec4(0.000f, 0.902f, 0.463f, 1.00f); // #00E676
    g_currentTheme.terminalYellow = ImVec4(1.000f, 0.839f, 0.000f, 1.00f); // #FFD600
    g_currentTheme.textPrimary    = ImVec4(0.973f, 0.980f, 0.988f, 1.00f); // #F8FAFC
    g_currentTheme.textSecondary  = ImVec4(0.580f, 0.639f, 0.722f, 1.00f); // #94A3B8
    g_currentTheme.textMuted      = ImVec4(0.392f, 0.455f, 0.545f, 1.00f); // #64748B
    g_currentTheme.btnText        = ImVec4(1.000f, 1.000f, 1.000f, 1.00f); // #FFFFFF
  } else {
    // Light Mode (Android values/colors.xml)
    g_currentTheme.bg             = ImVec4(0.973f, 0.980f, 0.988f, 1.00f); // #F8FAFC
    g_currentTheme.surface        = ImVec4(1.000f, 1.000f, 1.000f, 1.00f); // #FFFFFF
    g_currentTheme.card           = ImVec4(0.945f, 0.961f, 0.976f, 1.00f); // #F1F5F9
    g_currentTheme.cardBorder     = ImVec4(0.886f, 0.910f, 0.941f, 1.00f); // #E2E8F0
    g_currentTheme.alriRed        = ImVec4(0.898f, 0.035f, 0.078f, 1.00f); // #E50914
    g_currentTheme.alriRedDark    = ImVec4(0.698f, 0.027f, 0.063f, 1.00f); // #B20710
    g_currentTheme.alriRedGlow    = ImVec4(1.000f, 0.118f, 0.153f, 1.00f); // #FF1E27
    g_currentTheme.terminalGreen  = ImVec4(0.000f, 0.800f, 0.400f, 1.00f); // #00E676
    g_currentTheme.terminalYellow = ImVec4(0.850f, 0.680f, 0.000f, 1.00f); // #FFD600
    g_currentTheme.textPrimary    = ImVec4(0.059f, 0.090f, 0.165f, 1.00f); // #0F172A
    g_currentTheme.textSecondary  = ImVec4(0.278f, 0.333f, 0.412f, 1.00f); // #475569
    g_currentTheme.textMuted      = ImVec4(0.580f, 0.639f, 0.722f, 1.00f); // #94A3B8
    g_currentTheme.btnText        = ImVec4(1.000f, 1.000f, 1.000f, 1.00f); // #FFFFFF
  }

  ImGuiStyle &style = ImGui::GetStyle();
  ImVec4 *colors = style.Colors;

  style.WindowRounding    = 10.0f;
  style.ChildRounding     = 8.0f;
  style.FrameRounding     = 6.0f;
  style.PopupRounding     = 8.0f;
  style.ScrollbarRounding = 6.0f;
  style.GrabRounding      = 6.0f;
  style.TabRounding       = 6.0f;

  style.WindowBorderSize  = 1.0f;
  style.ChildBorderSize   = 1.0f;
  style.FrameBorderSize   = 1.0f;
  style.PopupBorderSize   = 1.0f;

  style.WindowPadding     = ImVec2(16.0f, 16.0f);
  style.FramePadding      = ImVec2(12.0f, 8.0f);
  style.ItemSpacing       = ImVec2(10.0f, 10.0f);
  style.ItemInnerSpacing  = ImVec2(8.0f, 6.0f);

  colors[ImGuiCol_Text]                  = g_currentTheme.textPrimary;
  colors[ImGuiCol_TextDisabled]          = g_currentTheme.textMuted;
  colors[ImGuiCol_WindowBg]              = g_currentTheme.bg;
  colors[ImGuiCol_ChildBg]               = g_currentTheme.card;
  colors[ImGuiCol_PopupBg]               = g_currentTheme.surface;
  colors[ImGuiCol_Border]                = g_currentTheme.cardBorder;
  colors[ImGuiCol_BorderShadow]          = ImVec4(0, 0, 0, 0);

  colors[ImGuiCol_FrameBg]               = g_currentTheme.surface;
  colors[ImGuiCol_FrameBgHovered]        = g_currentTheme.cardBorder;
  colors[ImGuiCol_FrameBgActive]         = g_currentTheme.cardBorder;

  colors[ImGuiCol_TitleBg]               = g_currentTheme.bg;
  colors[ImGuiCol_TitleBgActive]         = g_currentTheme.bg;
  colors[ImGuiCol_TitleBgCollapsed]      = g_currentTheme.bg;

  colors[ImGuiCol_MenuBarBg]             = g_currentTheme.card;
  colors[ImGuiCol_ScrollbarBg]           = g_currentTheme.bg;
  colors[ImGuiCol_ScrollbarGrab]         = g_currentTheme.cardBorder;
  colors[ImGuiCol_ScrollbarGrabHovered]  = g_currentTheme.textSecondary;
  colors[ImGuiCol_ScrollbarGrabActive]   = g_currentTheme.alriRed;

  colors[ImGuiCol_CheckMark]             = g_currentTheme.alriRed;
  colors[ImGuiCol_SliderGrab]            = g_currentTheme.alriRed;
  colors[ImGuiCol_SliderGrabActive]      = g_currentTheme.alriRedGlow;

  colors[ImGuiCol_Button]                = g_currentTheme.alriRed;
  colors[ImGuiCol_ButtonHovered]         = g_currentTheme.alriRedGlow;
  colors[ImGuiCol_ButtonActive]          = g_currentTheme.alriRedDark;

  colors[ImGuiCol_Header]                = g_currentTheme.card;
  colors[ImGuiCol_HeaderHovered]         = g_currentTheme.cardBorder;
  colors[ImGuiCol_HeaderActive]          = g_currentTheme.alriRed;

  colors[ImGuiCol_Separator]             = g_currentTheme.cardBorder;
  colors[ImGuiCol_SeparatorHovered]      = g_currentTheme.alriRed;
  colors[ImGuiCol_SeparatorActive]       = g_currentTheme.alriRedGlow;

  colors[ImGuiCol_ResizeGrip]            = ImVec4(0, 0, 0, 0);
  colors[ImGuiCol_ResizeGripHovered]     = g_currentTheme.cardBorder;
  colors[ImGuiCol_ResizeGripActive]      = g_currentTheme.alriRed;

  colors[ImGuiCol_Tab]                   = g_currentTheme.card;
  colors[ImGuiCol_TabHovered]            = g_currentTheme.alriRedGlow;
  colors[ImGuiCol_TabActive]             = g_currentTheme.alriRed;
  colors[ImGuiCol_TabUnfocused]          = g_currentTheme.card;
  colors[ImGuiCol_TabUnfocusedActive]    = g_currentTheme.cardBorder;
}

} // namespace GUI
