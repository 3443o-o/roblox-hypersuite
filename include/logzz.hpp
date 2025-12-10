#pragma once
#include <string>
#include <cstring>
#include <fstream>
#include <stdio.h>
#include <cstring>
#include <filesystem>
#include <thread>
#include <chrono>
#include <iostream>
#include <vector>
#include <map>

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#else
    #include <sys/stat.h>
        #if defined(__linux__)
            #include <fcntl.h>
        #endif
#endif

inline const unsigned long long CAMFIX_PLACEIDS[1] = {
    4597361034,
};

namespace fs = std::filesystem;

enum state {IN_GAME, IN_LUA_APP, OFFLINE, INVALID, UNCHANGED_FILE};

inline long long calculate_file_size_stat(const char *filepath) {
#ifdef _WIN32
    WIN32_FILE_ATTRIBUTE_DATA file_info;
    // GetFileAttributesExA is faster than CreateFile+GetFileSizeEx
    if (GetFileAttributesExA(filepath, GetFileExInfoStandard, &file_info)) {
        LARGE_INTEGER size;
        size.HighPart = file_info.nFileSizeHigh;
        size.LowPart = file_info.nFileSizeLow;
        return size.QuadPart;
    }
    return -1;
#else
    #if defined(__linux__)
        // statx is fastest on Linux (kernel 4.11+)
        struct statx stx;
        if (statx(AT_FDCWD, filepath, 0, STATX_SIZE, &stx) == 0) {
            return stx.stx_size;
        }
        // Fallback if statx not available
    #endif

    // stat64 for large file support on 32-bit systems
    #if defined(_LARGEFILE64_SOURCE) || defined(__USE_LARGEFILE64)
        struct stat64 file_info;
        if (stat64(filepath, &file_info) == 0) {
            return file_info.st_size;
        }
    #else
        struct stat file_info;
        if (stat(filepath, &file_info) == 0) {
            return file_info.st_size;
        }
    #endif

    return -1;
#endif
}


// Will return current state
inline namespace logzz {
    inline state current_state = OFFLINE;
    inline state last_state = UNCHANGED_FILE;
    inline std::string logs_folder_path;
    inline unsigned long long current_place_ID = 0;
    inline long long last_file_size;
    inline unsigned int camfix_proofs;
    inline bool game_uses_camfix_final;
    inline int game_uses_camfix_percentage;
    inline std::map<unsigned long long, int> calculated_placeIDs;

    //-- camfix shit

    inline state loop_handle() {
        logzz::last_state = logzz::current_state;
        std::string first_log_file;

        for (const auto & entry : fs::directory_iterator(logs_folder_path))
            first_log_file = entry.path().string();

        long long current_file_size = calculate_file_size_stat(first_log_file.c_str());
        if (current_file_size == last_file_size) return UNCHANGED_FILE;

        //log file changed.
        last_file_size = current_file_size;

        std::ifstream log_file(first_log_file);
        if (!log_file.is_open()) {
            printf("%s\n", "couldn't open file");
            return INVALID;
        };

        std::string last_place_id_string;
        int last_place_id_line = 0;

        int in_lua_app_line = 0;
        int left_roblox_line = 0;

        // cam fix detection
        int thistoweruses_line = 0;
        int setpartcollisiongroup_line = 0;
        int clientobjects_line = 0;
        int localpartsscripterror_line = 0;
        int workspaceobby_line = 0;
        int towerword_line = 0;
        int playerscripts_line = 0;

        std::string current_line;
        int line = 0;
        while (std::getline(log_file, current_line)) {
            line++;
            for (int i = 0; i < current_line.size(); i++) {
                {
                    size_t join_pos = current_line.find("Joining");
                        if (join_pos != std::string::npos) {
                            size_t placeid_index = join_pos + 58;
                            size_t place_id_end = current_line.find(' ', placeid_index);
                            if (place_id_end != std::string::npos) {
                                last_place_id_string = current_line.substr(placeid_index, place_id_end - placeid_index);
                                last_place_id_line = line;
                            }
                        }
                }

                if (current_line.find("returnToLuaApp") != std::string::npos) {
                    in_lua_app_line = line;
                }
                if (current_line.find("setStage: (stage:None)") != std::string::npos) {
                    left_roblox_line = line;
                }

                //-- CAMFIX DETECTION
                if (current_line.find("This tower uses") != std::string::npos) {
                    thistoweruses_line = line;
                }
                if (current_line.find("Warning: SetPartCollisionGroup is deprecated") != std::string::npos) {
                    setpartcollisiongroup_line = line;
                }
                if (current_line.find("ClientParts") != std::string::npos ||
                    current_line.find("ClientObject") != std::string::npos ||
                    current_line.find("ClientSidedObject") != std::string::npos ||
                    current_line.find("ClientObjectScript") != std::string::npos
                ) {
                    clientobjects_line = line;
                }
                if (current_line.find("LocalPartScript") != std::string::npos) {
                    localpartsscripterror_line = line;
                }
                if (current_line.find("PlayerScript") != std::string::npos) {
                    playerscripts_line = line;
                }
                if (current_line.find("Workspace.Obby") != std::string::npos) {
                    workspaceobby_line = line;
                }
                if (current_line.find("tower") != std::string::npos) {
                    towerword_line = line;
                }
            }
        };

        // states
        if (left_roblox_line > last_place_id_line && left_roblox_line > in_lua_app_line){
            current_state = OFFLINE;
        } else if (in_lua_app_line > last_place_id_line) {
            current_state = IN_LUA_APP;
        } else if (in_lua_app_line < last_place_id_line) {
            current_state = IN_GAME;
            if (!last_place_id_string.empty()) {
                current_place_ID = std::stoull(last_place_id_string);
            } else {
                current_place_ID = 0;
            }
        } else {
            current_state = OFFLINE;
        }

        log_file.close();

        //CAM FIX DETECTION
        if (current_state == IN_GAME && current_place_ID > 0) {
            if (calculated_placeIDs[current_place_ID]) return current_state;
            int score = 0;

            if (thistoweruses_line > last_place_id_line) score += 200;
            if (setpartcollisiongroup_line > last_place_id_line) score += 20;
            if (clientobjects_line > last_place_id_line) score += 100;
            if (localpartsscripterror_line > last_place_id_line) score += 10;
            if (workspaceobby_line > last_place_id_line) score += 20;
            if (towerword_line > last_place_id_line) score += 20;
            if (playerscripts_line > last_place_id_line) score += 30;

            calculated_placeIDs[current_place_ID] = (int)((score / 300.0f) * 100);
            if (calculated_placeIDs[current_place_ID] > 0) printf("CAMFIX POSSIBILTY: %d % \n", calculated_placeIDs[current_place_ID]);
        }
        return current_state;
    }
}
