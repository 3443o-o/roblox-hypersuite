#pragma once
#include <fstream>
#include <map>
#include <string>
#include "json.hpp"
#include "inpctrl.hpp"
#include "Globals.hpp"
#include "Speedglitch.hpp"
#include "LagSwitch.hpp"
#include "UserInterface.hpp"
#include "imgui.h"

using json = nlohmann::json;
const char* SETTINGS_FILE = "saved.json";

inline namespace SettingsHandler {

    inline void SaveSettings() {
        json j;

        //-- Saving keybinds
        for (const auto& [name, key] : Binds) {
            j["binds"][name] = static_cast<unsigned int>(key);
        }

        j["ChatKey"] = static_cast<unsigned int>(ChatKey);
        j["SpamKey"] = static_cast<unsigned int>(SpamKey);

        //-- Settings tab
        j["roblox_process_name"] = roblox_process_name;
        j["roblox_sensitivity"] = roblox_sensitivity;
        j["cam_fix_active"] = cam_fix_active;
        j["roblox_fps"] = roblox_fps;
        j["kb_layout"] = kb_layout;

        //-- Saves state
        for (int i = 0; i < sizeof(enabled) / sizeof(enabled[0]); i++) {
            j["enabled"][std::to_string(i)] = enabled[i];
        }

        //-- Theme
        j["themeColor"] = {
            {"r", themeColor.x},
            {"g", themeColor.y},
            {"b", themeColor.z},
            {"a", themeColor.w}
        };
        j["rainbowThemeEnabled"] = rainbowThemeEnabled;
        j["rainbowHue"] = rainbowHue;
        j["rainbowSpeed"] = rainbowSpeed;
        j["rainbowSaturation"] = rainbowSaturation;
        j["rainbowValue"] = rainbowValue;
        j["resizable_window"] = resizable_window;
        j["decorated_window"] = decorated_window;

        std::ofstream file(SETTINGS_FILE);
        file << j.dump(4);
    }

    inline void LoadSettings() {
        std::ifstream file(SETTINGS_FILE);
        if (!file.is_open())
            return;

        json j;
        file >> j;

        //-- Loading keybinds
        if (j.contains("binds")) {
            for (auto& [name, value] : j["binds"].items()) {
                Binds[name] = static_cast<CrossInput::Key>(value.get<unsigned int>());
            }
        }

        if (j.contains("ChatKey"))
            ChatKey = static_cast<CrossInput::Key>(j["ChatKey"].get<unsigned int>());

        if (j.contains("SpamKey"))
            SpamKey = static_cast<CrossInput::Key>(j["SpamKey"].get<unsigned int>());

        if (j.contains("roblox_process_name"))
            roblox_process_name = j["roblox_process_name"];

        if (j.contains("cam_fix_active"))
            cam_fix_active = j["cam_fix_active"];

        if (j.contains("roblox_sensitivity"))
            updateSpeedglitchSensitivity(j["roblox_sensitivity"], cam_fix_active);

        if (j.contains("roblox_fps"))
            updateSpeedglitchFPS(j["roblox_fps"]);

        if (j.contains("kb_layout"))
            kb_layout = j["kb_layout"];

        // -- Load enabled array
        if (j.contains("enabled")) {
            for (int i = 0; i < 12; i++) {
                std::string idx = std::to_string(i);
                if (j["enabled"].contains(idx)) {
                    enabled[i] = j["enabled"][idx].get<bool>();
                }
            }
        }

        if (j.contains("themeColor")) {
            themeColor = ImVec4(
                j["themeColor"]["r"].get<float>(),
                j["themeColor"]["g"].get<float>(),
                j["themeColor"]["b"].get<float>(),
                j["themeColor"]["a"].get<float>()
            );
        }
        if (j.contains("rainbowThemeEnabled"))
            rainbowThemeEnabled = j["rainbowThemeEnabled"].get<bool>();

        if (j.contains("rainbowHue"))
            rainbowHue = j["rainbowHue"].get<float>();

        if (j.contains("rainbowSpeed"))
            rainbowSpeed = j["rainbowSpeed"].get<float>();

        if (j.contains("rainbowSaturation"))
            rainbowSaturation = j["rainbowSaturation"].get<float>();

        if (j.contains("resizable_window")) {
            resizable_window = j["resizable_window"].get<float>();
            lastResizable = resizable_window;
        }

        if (j.contains("decorated_window")) {
            decorated_window = j["decorated_window"];
            lastDecorated = decorated_window;
        }
    }
}
