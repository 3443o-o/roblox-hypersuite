#pragma once

#include <unordered_map>
#include <string>
#include <cstdlib>
#include <stdio.h>
#include <iostream>
#include <vector>

#ifndef _WIN32
#include <pwd.h>
#include <unistd.h>
#endif

#include "pugixml.hpp"
#include "Globals.hpp"
#include "Helper.hpp"
#include "RobloxFiles.hpp"

inline std::string GlobalBasicSettingsFile = "empty";
inline char gbsPathBuffer[512] = "";

// Setting types
enum class SettingType {
    BOOL,
    INT,
    FLOAT,
    TOKEN,
    STRING,
    VECTOR2
};

struct SettingValue {
    SettingType type;
    std::string name;

    // Value storage
    bool boolVal = false;
    int intVal = 0;
    float floatVal = 0.0f;
    std::string stringVal = "";
    float vec2X = 0.0f;
    float vec2Y = 0.0f;
};

inline std::vector<SettingValue> cachedSettings;

inline void setGBSFileDirectory() {
#ifdef _WIN32
    const char* localAppData = std::getenv("LOCALAPPDATA");
    if (localAppData) {
        GlobalBasicSettingsFile = std::string(localAppData) +
            "\\Roblox\\GlobalBasicSettings_13.xml";
    } else {
        GlobalBasicSettingsFile = "C:\\Users\\Default\\AppData\\Local\\Roblox\\GlobalBasicSettings_13.xml";
    }
#else
    // Linux / Flatpak
    const char* home = nullptr;

    // If running under sudo, get the original user's home directory
    const char* sudoUser = std::getenv("SUDO_USER");
    if (sudoUser) {
        struct passwd* pw = getpwnam(sudoUser);
        if (pw) home = pw->pw_dir;
    }

    // Fallback to normal HOME
    if (!home) home = std::getenv("HOME");

    // Fallback if all else fails
    if (!home) home = "/home/username";

    GlobalBasicSettingsFile = std::string(home) +
        "/.var/app/org.vinegarhq.Sober/data/sober/appData/GlobalBasicSettings_13.xml";
#endif
}

inline void loadAllSettings() {
    cachedSettings.clear();

    pugi::xml_document doc;
    if (!doc.load_file(GlobalBasicSettingsFile.c_str())) {
        log("Failed to load XML: " + GlobalBasicSettingsFile);
        return;
    }

    pugi::xml_node root = doc.child("roblox");
    pugi::xml_node item = root.find_child_by_attribute("Item", "class", "UserGameSettings");
    if (!item) {
        log("UserGameSettings not found");
        return;
    }

    pugi::xml_node props = item.child("Properties");
    if (!props) {
        log("Properties node not found");
        return;
    }

    // Parse all settings
    for (pugi::xml_node node : props.children()) {
        SettingValue setting;
        std::string type = node.name();
        setting.name = node.attribute("name").as_string();

        if (type == "bool") {
            setting.type = SettingType::BOOL;
            setting.boolVal = node.text().as_bool();
            cachedSettings.push_back(setting);
        }
        else if (type == "int") {
            setting.type = SettingType::INT;
            setting.intVal = node.text().as_int();
            cachedSettings.push_back(setting);
        }
        else if (type == "float") {
            setting.type = SettingType::FLOAT;
            setting.floatVal = node.text().as_float();
            cachedSettings.push_back(setting);
        }
        else if (type == "token") {
            setting.type = SettingType::TOKEN;
            setting.intVal = node.text().as_int();
            cachedSettings.push_back(setting);
        }
        else if (type == "string") {
            setting.type = SettingType::STRING;
            setting.stringVal = node.text().as_string();
            cachedSettings.push_back(setting);
        }
        else if (type == "Vector2") {
            setting.type = SettingType::VECTOR2;
            pugi::xml_node xNode = node.child("X");
            pugi::xml_node yNode = node.child("Y");
            if (xNode && yNode) {
                setting.vec2X = xNode.text().as_float();
                setting.vec2Y = yNode.text().as_float();
                cachedSettings.push_back(setting);
            }
        }
    }

    log("Loaded " + std::to_string(cachedSettings.size()) + " settings");
}

inline bool saveAllSettings() {
    pugi::xml_document doc;
    if (!doc.load_file(GlobalBasicSettingsFile.c_str())) {
        log("Failed to load XML: " + GlobalBasicSettingsFile);
        return false;
    }

    pugi::xml_node root = doc.child("roblox");
    pugi::xml_node item = root.find_child_by_attribute("Item", "class", "UserGameSettings");
    if (!item) {
        log("UserGameSettings not found");
        return false;
    }

    pugi::xml_node props = item.child("Properties");
    if (!props) {
        log("Properties node not found");
        return false;
    }

    // Update all settings
    for (const auto& setting : cachedSettings) {
        bool found = false;

        for (pugi::xml_node node : props.children()) {
            std::string nodeName = node.attribute("name").as_string();
            if (nodeName == setting.name) {
                found = true;

                switch (setting.type) {
                    case SettingType::BOOL:
                        node.text() = setting.boolVal;
                        break;
                    case SettingType::INT:
                    case SettingType::TOKEN:
                        node.text() = setting.intVal;
                        break;
                    case SettingType::FLOAT:
                        node.text() = setting.floatVal;
                        break;
                    case SettingType::STRING:
                        node.text() = setting.stringVal.c_str();
                        break;
                    case SettingType::VECTOR2:
                        node.child("X").text() = setting.vec2X;
                        node.child("Y").text() = setting.vec2Y;
                        break;
                }
                break;
            }
        }
    }

    if (!doc.save_file(GlobalBasicSettingsFile.c_str())) {
        log("Failed to save XML");
        return false;
    }

    log("All settings saved successfully");
    return true;
}

// Legacy compatibility functions
inline float GetGBSValue(const std::string& key) {
    for (const auto& setting : cachedSettings) {
        if (setting.name == key) {
            switch (setting.type) {
                case SettingType::BOOL: return setting.boolVal ? 1.0f : 0.0f;
                case SettingType::INT:
                case SettingType::TOKEN: return static_cast<float>(setting.intVal);
                case SettingType::FLOAT: return setting.floatVal;
                default: return 0.0f;
            }
        }
    }

    log("Key not found in cache: " + key);
    return 0.0f;
}

inline void setGBSFramerateCap(int newFPS) {
    for (auto& setting : cachedSettings) {
        if (setting.name == "FramerateCap") {
            setting.intVal = newFPS;
            saveAllSettings();
            log("FramerateCap changed to " + std::to_string(newFPS));
            return;
        }
    }
    log("FramerateCap not found in cache");
}

inline void renderRobloxSettingsWindow() {
    if (ImGui::BeginTabItem("Roblox")) {
        // Use default path if not set
        if (GlobalBasicSettingsFile == "empty") {
            setGBSFileDirectory();
        }

        // ===== QUICK SETTINGS SECTION =====
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.9f, 1.0f, 1.0f));
        ImGui::Text("Quick Settings");
        ImGui::PopStyleColor();
        ImGui::Separator();
        ImGui::Spacing();

        // Framerate Cap Setting
        static int quickFPS = 60;
        static bool fpsLoaded = false;
        static bool showFpsSuccess = false;
        static float fpsMessageTimer = 0.0f;

        if (!fpsLoaded && !cachedSettings.empty()) {
            quickFPS = static_cast<int>(GetGBSValue("FramerateCap"));
            fpsLoaded = true;
        }

        ImGui::AlignTextToFramePadding();
        ImGui::Text("FPS Cap:");
        ImGui::SameLine(80);
        ImGui::SetNextItemWidth(80);
        ImGui::InputInt("##QuickFPS", &quickFPS);
        ImGui::SameLine();

        if (ImGui::Button("Apply##FPS", ImVec2(60, 0))) {
            if (cachedSettings.empty()) {
                loadAllSettings();
            }
            setGBSFramerateCap(quickFPS);
            fpsLoaded = false;
            showFpsSuccess = true;
            fpsMessageTimer = 3.0f;
            log("Framerate cap applied: " + std::to_string(quickFPS));
        }

        if (showFpsSuccess && fpsMessageTimer > 0.0f) {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "Applied!");
            fpsMessageTimer -= ImGui::GetIO().DeltaTime;
            if (fpsMessageTimer <= 0.0f) {
                showFpsSuccess = false;
            }
        }

        ImGui::Spacing();

        // Roblox Control Section
        static bool wasRunningBeforeRestart = false;
        static bool isRestarting = false;
        static bool showRestartSuccess = false;
        static float restartMessageTimer = 0.0f;
        static std::string restartStatusMsg = "";

        static bool showPlaceFetchMsg = false;
        static bool placeFetchSuccess = false;
        static float placeFetchTimer = 0.0f;

        static bool showInstanceFetchMsg = false;
        static bool instanceFetchSuccess = false;
        static float instanceFetchTimer = 0.0f;

        ImGui::AlignTextToFramePadding();
        ImGui::Text("Place ID:");
        ImGui::SameLine(80);
        ImGui::SetNextItemWidth(150);
        ImGui::InputText("##PlaceID", placeIdBuffer, sizeof(placeIdBuffer));
        ImGui::SameLine();
        if (ImGui::Button("Fetch##PlaceID", ImVec2(60, 0))) {
            unsigned long long lastPlaceID = getLastPlaceID();
            if (lastPlaceID != 0) {
                std::string placeIDStr = std::to_string(lastPlaceID);
                std::snprintf(placeIdBuffer, sizeof(placeIdBuffer), "%s", placeIDStr.c_str());
                placeFetchSuccess = true;
            } else {
                placeFetchSuccess = false;
            }
            showPlaceFetchMsg = true;
            placeFetchTimer = 3.0f;
        }

        if (showPlaceFetchMsg && placeFetchTimer > 0.0f) {
            ImGui::SameLine();
            if (placeFetchSuccess) {
                ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "Fetched!");
            } else {
                ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Failed!");
            }
            placeFetchTimer -= ImGui::GetIO().DeltaTime;
            if (placeFetchTimer <= 0.0f) {
                showPlaceFetchMsg = false;
            }
        }

        ImGui::AlignTextToFramePadding();
        ImGui::Text("Instance:");
        ImGui::SameLine(80);
        ImGui::SetNextItemWidth(150);

        // Disable instance ID if no place ID
        bool hasPlaceId = strlen(placeIdBuffer) > 0;
        if (!hasPlaceId) {
            ImGui::BeginDisabled();
        }

        ImGui::InputText("##InstanceID", instanceIdBuffer, sizeof(instanceIdBuffer));
        ImGui::SameLine();
        if (ImGui::Button("Fetch##InstanceID", ImVec2(60, 0))) {
            std::string lastInstanceID = getLastInstanceID();
            if (!lastInstanceID.empty()) {
                std::snprintf(instanceIdBuffer, sizeof(instanceIdBuffer), "%s", lastInstanceID.c_str());
                instanceFetchSuccess = true;
            } else {
                instanceFetchSuccess = false;
            }
            showInstanceFetchMsg = true;
            instanceFetchTimer = 3.0f;
        }

        if (!hasPlaceId) {
            ImGui::EndDisabled();
        }

        if (showInstanceFetchMsg && instanceFetchTimer > 0.0f) {
            ImGui::SameLine();
            if (instanceFetchSuccess) {
                ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "Fetched!");
            } else {
                ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Failed!");
            }
            instanceFetchTimer -= ImGui::GetIO().DeltaTime;
            if (instanceFetchTimer <= 0.0f) {
                showInstanceFetchMsg = false;
            }
        }

        ImGui::Spacing();

        if (ImGui::Button("Restart / Start Roblox", ImVec2(160, 0))) {
            // Check BEFORE restarting to show correct message
            wasRunningBeforeRestart = isProcessRunning(roblox_process_name);
            isRestarting = true;
            restartStatusMsg = wasRunningBeforeRestart ? "Restarting Roblox..." : "Starting Roblox...";
            restartMessageTimer = 0.0f;
            showRestartSuccess = true;

            // Call restart function
            restartRoblox();
        }

        // Handle restart status messages
        if (isRestarting) {
            restartMessageTimer += ImGui::GetIO().DeltaTime;

            // Check if process is running after a brief delay
            if (restartMessageTimer > 2.0f) {
                if (isProcessRunning(roblox_process_name)) {
                    restartStatusMsg = wasRunningBeforeRestart ? "Roblox restarted!" : "Roblox started!";
                    isRestarting = false;
                    restartMessageTimer = 3.0f; // Show success for 3 seconds
                }
            }
        }

        if (showRestartSuccess && restartMessageTimer > 0.0f) {
            if (isRestarting) {
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "%s", restartStatusMsg.c_str());
            } else {
                ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "%s", restartStatusMsg.c_str());
                restartMessageTimer -= ImGui::GetIO().DeltaTime;
                if (restartMessageTimer <= 0.0f) {
                    showRestartSuccess = false;
                }
            }
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // ===== ADVANCED SETTINGS SECTION =====
        if (ImGui::CollapsingHeader("Global basic settings editor")) {
            ImGui::Spacing();

            // Config Path
            ImGui::Text("Custom Config Path:");
            ImGui::SetNextItemWidth(350);
            ImGui::InputText("##GBSPath", gbsPathBuffer, sizeof(gbsPathBuffer));
            ImGui::SameLine();

            bool hasCustomPath = strlen(gbsPathBuffer) > 0;

            if (ImGui::Button(hasCustomPath ? "Apply##Path" : "Default##Path", ImVec2(60, 0))) {
                if (hasCustomPath) {
                    GlobalBasicSettingsFile = std::string(gbsPathBuffer);
                    log("Custom GBS path set: " + GlobalBasicSettingsFile);
                } else {
                    setGBSFileDirectory(); // Load default path
                    log("Loaded default GBS path");
                }
            }

            if (!hasCustomPath) {
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Leave empty to use default path");
            }

            ImGui::Spacing();

            // Settings Management Buttons
            if (ImGui::Button("Load", ImVec2(70, 0))) {
                loadAllSettings();
            }
            ImGui::SameLine();
            if (ImGui::Button("Save", ImVec2(70, 0))) {
                saveAllSettings();
            }
            ImGui::SameLine();
            if (ImGui::Button("Reload", ImVec2(70, 0))) {
                loadAllSettings();
            }
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "(%zu)", cachedSettings.size());

            ImGui::Spacing();

            // Search filter
            static char searchBuffer[256] = "";
            ImGui::Text("Search:");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(300);
            ImGui::InputTextWithHint("##Search", "Filter...", searchBuffer, sizeof(searchBuffer));
            std::string searchStr = searchBuffer;

            ImGui::Spacing();

            // Settings List (compact for 500x400 window)
            if (ImGui::BeginChild("SettingsList", ImVec2(0, 150), true)) {
                bool hasResults = false;

                for (auto& setting : cachedSettings) {
                    // Filter by search
                    if (!searchStr.empty() &&
                        setting.name.find(searchStr) == std::string::npos) {
                        continue;
                    }

                    hasResults = true;
                    ImGui::PushID(setting.name.c_str());

                    switch (setting.type) {
                        case SettingType::BOOL:
                            ImGui::Checkbox(setting.name.c_str(), &setting.boolVal);
                            break;

                        case SettingType::INT:
                            ImGui::AlignTextToFramePadding();
                            ImGui::Text("%s", setting.name.c_str());
                            ImGui::SameLine(220);
                            ImGui::SetNextItemWidth(100);
                            ImGui::InputInt(("##" + setting.name).c_str(), &setting.intVal);
                            break;

                        case SettingType::TOKEN:
                            ImGui::AlignTextToFramePadding();
                            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.4f, 1.0f), "%s", setting.name.c_str());
                            ImGui::SameLine(220);
                            ImGui::SetNextItemWidth(100);
                            ImGui::InputInt(("##" + setting.name).c_str(), &setting.intVal);
                            break;

                        case SettingType::FLOAT:
                            ImGui::AlignTextToFramePadding();
                            ImGui::Text("%s", setting.name.c_str());
                            ImGui::SameLine(220);
                            ImGui::SetNextItemWidth(100);
                            ImGui::InputFloat(("##" + setting.name).c_str(), &setting.floatVal, 0.01f, 0.1f, "%.2f");
                            break;

                        case SettingType::STRING: {
                            ImGui::AlignTextToFramePadding();
                            ImGui::Text("%s", setting.name.c_str());
                            ImGui::SameLine(220);
                            ImGui::SetNextItemWidth(180);
                            char buffer[512];
                            strncpy(buffer, setting.stringVal.c_str(), sizeof(buffer) - 1);
                            buffer[sizeof(buffer) - 1] = '\0';
                            if (ImGui::InputText(("##" + setting.name).c_str(), buffer, sizeof(buffer))) {
                                setting.stringVal = buffer;
                            }
                            break;
                        }

                        case SettingType::VECTOR2:
                            ImGui::AlignTextToFramePadding();
                            ImGui::Text("%s", setting.name.c_str());
                            ImGui::SameLine(220);
                            ImGui::SetNextItemWidth(80);
                            ImGui::InputFloat(("##X" + setting.name).c_str(), &setting.vec2X, 0.01f, 0.1f, "%.1f");
                            ImGui::SameLine();
                            ImGui::SetNextItemWidth(80);
                            ImGui::InputFloat(("##Y" + setting.name).c_str(), &setting.vec2Y, 0.01f, 0.1f, "%.1f");
                            break;
                    }

                    ImGui::PopID();
                }

                if (!hasResults && !searchStr.empty()) {
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No results for '%s'", searchStr.c_str());
                }
            }
            ImGui::EndChild();
        }

        ImGui::EndTabItem();
    }
}
