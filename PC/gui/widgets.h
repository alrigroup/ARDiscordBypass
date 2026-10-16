#pragma once

#include "vendor/imgui/imgui.h"
#include <cmath>
#include <string>

namespace GUI {

inline void DrawPlayIcon(ImDrawList *drawList, ImVec2 pos, float size, ImU32 color) {
  ImVec2 p1 = pos;
  ImVec2 p2 = ImVec2(pos.x + size, pos.y + size * 0.5f);
  ImVec2 p3 = ImVec2(pos.x, pos.y + size);
  drawList->AddTriangleFilled(p1, p2, p3, color);
}

inline void DrawStopIcon(ImDrawList *drawList, ImVec2 pos, float size, ImU32 color) {
  drawList->AddRectFilled(pos, ImVec2(pos.x + size, pos.y + size), color, 3.0f);
}

inline void DrawPulsingStatusDot(ImDrawList *drawList, ImVec2 center, float radius, ImU32 coreColor, bool isPulsing) {
  drawList->AddCircleFilled(center, radius, coreColor, 20);

  if (isPulsing) {
    float t = static_cast<float>(ImGui::GetTime());
    float wave = (std::sin(t * 3.5f) + 1.0f) * 0.5f;
    float ringR = radius + 2.0f + wave * 6.0f;
    int alpha = static_cast<int>((1.0f - wave) * 160.0f);
    ImU32 ringColor = (coreColor & 0x00FFFFFF) | (static_cast<ImU32>(alpha) << 24);
    drawList->AddCircle(center, ringR, ringColor, 24, 1.5f);
  }
}

inline void DrawRotatingSpinner(ImDrawList *drawList, ImVec2 center, float radius, float thickness, ImU32 color) {
  float t = static_cast<float>(ImGui::GetTime());
  float startAngle = t * 6.0f;
  float sweepAngle = 1.5f * 3.14159265f;
  drawList->PathClear();
  drawList->PathArcTo(center, radius, startAngle, startAngle + sweepAngle, 28);
  drawList->PathStroke(color, 0, thickness);
}

inline void DrawIndeterminateProgressBar(ImDrawList *drawList, ImVec2 min, ImVec2 max, ImU32 bgColor, ImU32 barColor) {
  drawList->AddRectFilled(min, max, bgColor, 4.0f);
  float w = max.x - min.x;
  float t = std::fmod(static_cast<float>(ImGui::GetTime()) * 0.85f, 1.0f);
  float barW = w * 0.38f;
  float x0 = min.x + (w + barW) * t - barW;
  float x1 = x0 + barW;
  if (x0 < min.x) x0 = min.x;
  if (x1 > max.x) x1 = max.x;
  if (x1 > x0) {
    drawList->AddRectFilled(ImVec2(x0, min.y), ImVec2(x1, max.y), barColor, 4.0f);
  }
}

inline void DrawThemeIcon(ImDrawList *drawList, ImVec2 center, float radius, bool isDark, ImU32 color) {
  if (isDark) {
    // Lua crescente (Crescent Moon)
    drawList->AddCircleFilled(center, radius, color, 20);
    ImVec2 cutoutCenter = ImVec2(center.x + radius * 0.45f, center.y - radius * 0.25f);
    ImU32 cutColor = ImGui::GetColorU32(ImGuiCol_Button);
    drawList->AddCircleFilled(cutoutCenter, radius * 0.85f, cutColor, 20);
  } else {
    // Sol com raios (Sun)
    drawList->AddCircleFilled(center, radius * 0.55f, color, 16);
    const int numRays = 8;
    for (int i = 0; i < numRays; ++i) {
      float angle = i * (2.0f * 3.14159265f / numRays);
      ImVec2 p1 = ImVec2(center.x + std::cos(angle) * (radius * 0.75f), center.y + std::sin(angle) * (radius * 0.75f));
      ImVec2 p2 = ImVec2(center.x + std::cos(angle) * (radius * 1.05f), center.y + std::sin(angle) * (radius * 1.05f));
      drawList->AddLine(p1, p2, color, 1.5f);
    }
  }
}

} // namespace GUI
