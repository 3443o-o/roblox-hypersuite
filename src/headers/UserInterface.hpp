#pragma once
#include <string>
#include "imgui.h"

inline std::string current_option = "0";
inline std::string current_top_panel = "Macros";
inline bool rainbowThemeEnabled = false;
inline float rainbowHue = 0.0f;
inline float rainbowSpeed = 0.002f;
inline float rainbowSaturation = 0.85f;
inline float rainbowValue = 0.6f;

void initUI();
void UpdateUI();
void applyThemeColor(const ImVec4& color);