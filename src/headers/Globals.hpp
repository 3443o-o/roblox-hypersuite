#pragma once

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

// Rename Windows functions BEFORE including windows.h
#define Rectangle Win32Rectangle
#define CloseWindow Win32CloseWindow
#define ShowCursor Win32ShowCursor
#define DrawText Win32DrawText
#define DrawTextEx Win32DrawTextEx
#define LoadImage Win32LoadImage

#include <windows.h>
#include <shellapi.h>

// Restore original names AFTER windows.h
#undef Rectangle
#undef CloseWindow
#undef ShowCursor
#undef DrawText
#undef DrawTextEx
#undef LoadImage
#endif
#include <string>
#include <thread>
#include <map>
#include "imgui.h"
#include "netctrl.hpp"
#include "inpctrl.hpp"

// Inner-workings
inline netctrl::NetCtrl* g_ctrl = nullptr;
inline netctrl::NetCtrl ctrl;
inline bool is_elevated;
inline CrossInput input;
inline char passwordBuffer[256] = "";
inline bool elevationFailed = false;
inline bool decorated_window = false;
inline bool resizable_window = false;
inline bool windowOnTop = false;

inline bool lastDecorated = decorated_window;
inline bool lastResizable = resizable_window;
inline bool lastWindowOnTop = windowOnTop;

inline float screen_width;
inline float screen_height;

// Macro specific
//-- Speed glitch
inline std::atomic<bool> speedglitch_active(false);
inline std::thread speedglitch_thread;

//-- Spam button
inline CrossInput::Key SpamKey = CrossInput::Key::Num1;

//-- Helicopter High jump
inline int hhj_length = 243;
inline int hhj_freeze_delay = 0;
inline int hhj_delay1 = 9;
inline int hhj_delay2 = 17;
inline int hhj_delay3 = 16;
inline bool hhj_auto_timing = false;
inline bool hhj_fast_mode = false;
inline bool globalzoomin = false;  // If not already defined
inline std::atomic<bool> hhj_speedglitch_active(false);
inline std::thread hhj_speedglitch_thread;

// Settings
inline int speed_pixels_x = 716;  // Default for 0.5 sensitivity without cam-fix
inline int speed_pixels_y = -716; // Negative for opposite direction
inline int roblox_fps = 60;       // User's Roblox FPS
inline float roblox_sensitivity = 0.5f;
inline bool cam_fix_active = false;
inline ImVec4 themeColor = ImVec4(0.8f, 0.1f, 0.1f, 1.0f); // Default red theme
inline std::string roblox_process_name;
inline CrossInput::Key ChatKey = CrossInput::Key::Slash;

// Roblox tab
inline char placeIdBuffer[32] = "";  // buffer for Place ID input
inline char instanceIdBuffer[64] = "";  // buffer for Instance ID (UUID format)

// Debounces
inline bool events[16] {
    false, // Freeze
    false, // Laugh clip
    false, // E-Dance clip
    false, // For binding/changing keys
    false, // For lag switch
    false, // buckey clip
    false, // Filler - do not use
    false, // gear clip
    false, // gear clip bind
    false, // variable bind
    false, // Laugh Disable head collision
    false, // nhc roof clip
    false, // helicopter high jump
    false, // gear desync
    false, // full gear desync
    false, // floor bounce hj
};

inline bool enabled[16] {
    true, // Freeze
    true, // Laugh clip
    false, // E-Dance clip
    false, // align
    false, // For lag switch
    false, // buckey clip
    false, // Speed glitch
    false, // Gear clip
    false, // align / filler
    false, // align / filler
    false, // disable head collision
    false, // NHC roof clip
    false, // helicopter high jump
    false, // gear desync
    false, // full gear desync
    true, // floor bounce high jump
};

inline std::map<std::string, CrossInput::Key> Binds = {
    {"Freeze", CrossInput::Key::F1}, // Freeze
    {"Laugh", CrossInput::Key::F2}, // Laugh clip
    {"E-Dance", CrossInput::Key::F3}, // Extended Dance
    {"Lag-switch", CrossInput::Key::F4}, // Lag switch
    {"Buckey-clip", CrossInput::Key::F5}, // Buckey clip
    {"Speedglitch", CrossInput::Key::F6}, // Speed glitch
    {"Spam-Key", CrossInput::Key::F7}, // Gear clip
    {"Disable-Head-Collision", CrossInput::Key::F8}, // No head collision
    {"NHC-Roof", CrossInput::Key::Num9}, // NHC roof clip
    {"HHJ", CrossInput::Key::Num8}, // Helicopter high jump
    {"Gear-Desync", CrossInput::Key::Num7}, // Gear desync
    {"Full-Gear-Desync", CrossInput::Key::Numpad0}, // Full Gear desync
    {"Floor-Bounce-High-Jump", CrossInput::Key::Numpad1}, // floor bounce hj
};

inline unsigned short kb_layout;
inline const char* string_kb_layouts[] = {"US", "FR"};
