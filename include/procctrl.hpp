/* Hello!!

This is a simple, cross-platform, header-only library to suspend and resume processes written in C++. (I might rewrite this in C later.)
It currently supports Windows and Linux (fuck macos, never touching that shit).

Features:
- Suspend/resume a single process by PID
- Suspend/resume all processes by executable name
- Query if a process exists and can be controlled
- Find the parent PID or all descendants (process tree)
- Works on sandboxed applications on Linux (e.g., Snap/Flatpak)
- Automatically handles cgroups on Linux when possible
- Pure header-only: just include `procctrl.hpp` and use

How it works:

-- Windows:
- Uses the ntdll.dll functions (`NtSuspendProcess` / `NtResumeProcess`)
- Attempts to enable `SeDebugPrivilege` to control other users' processes
- Falls back gracefully if permissions are insufficient

-- Linux:
- Non-sandboxed apps: sends `SIGSTOP` to suspend and `SIGCONT` to resume
- Sandboxed apps in cgroup v2: writes `1` or `0` to `cgroup.freeze` for freezing/thawing
- Traverses `/proc` to find processes and parent/child relationships
- Supports cgroup-aware suspension to prevent partial freezes for multi-process applications

Huge thanks to the original inspirations:
- https://github.com/craftwar/suspend
- https://github.com/Spencer0187/Spencer-Macro-Utilities/tree/main/visual%20studio/Resource%20Files/Suspend_Input_Helper_Source

This project is based on the above but refactored into a single, easy-to-use, cross-platform header.

Usage example:
(C++)

----------------------------------------------------------------------
#include "procctrl.hpp"

int main() {
    auto pid = procctrl::find_process_by_name("firefox");
    if (pid != -1) {
        procctrl::set_process_suspended(pid, true);  // suspend
        procctrl::set_process_suspended(pid, false); // resume
    }
}
----------------------------------------------------------------------

*/


#pragma once
#include <string>
#include <vector>
#include <unordered_set>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdio>
#include <cerrno>
#include <cstring>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN // Reduces Windows header bloat
    #include <windows.h>
    #include <psapi.h>
    #include <tlhelp32.h>
    // Don't redefine pid_t if it already exists (MinGW defines it)
    #ifndef _PID_T_
        typedef DWORD pid_t;
    #endif
#undef Rectangle
#undef CloseWindow  
#undef ShowCursor
#else
    #include <csignal>
    #include <unistd.h>
    #include <dirent.h>
    #include <sys/stat.h>
#endif

namespace procctrl {

#ifdef _WIN32
// Windows implementation using undocumented NT API
typedef LONG(NTAPI *NtSuspendProcess)(HANDLE ProcessHandle);
typedef LONG(NTAPI *NtResumeProcess)(HANDLE ProcessHandle);

static NtSuspendProcess g_pfnNtSuspendProcess = nullptr;
static NtResumeProcess g_pfnNtResumeProcess = nullptr;

/// Enable SeDebugPrivilege to allow suspending processes owned by other users
/// @return true if privilege was enabled successfully
inline bool enable_debug_privilege() {
    HANDLE hToken;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
        return false;
    }
    
    TOKEN_PRIVILEGES tp;
    LUID luid;
    
    if (!LookupPrivilegeValueA(nullptr, "SeDebugPrivilege", &luid)) {
        CloseHandle(hToken);
        return false;
    }
    
    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    
    bool success = AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), nullptr, nullptr) != 0;
    CloseHandle(hToken);
    return success;
}

inline void init_nt_functions() {
    static bool initialized = false;
    if (!initialized) {
        HMODULE hNtdll = GetModuleHandleA("ntdll");
        if (hNtdll) {
            g_pfnNtSuspendProcess = reinterpret_cast<NtSuspendProcess>(
                GetProcAddress(hNtdll, "NtSuspendProcess"));
            g_pfnNtResumeProcess = reinterpret_cast<NtResumeProcess>(
                GetProcAddress(hNtdll, "NtResumeProcess"));
        }
        // Try to enable debug privilege (may fail if not admin, but that's okay)
        enable_debug_privilege();
        initialized = true;
    }
}
#else
/// Check if cgroup v2 is available on the system (Linux only)
/// @return true if cgroup v2 filesystem is mounted
inline bool is_cgroup_v2_available() {
    struct stat st;
    return (stat("/sys/fs/cgroup/cgroup.controllers", &st) == 0);
}

/// Get the maximum PID value configured on the system (Linux only)
/// @return Maximum PID value (default 32768 if cannot read)
inline int get_max_pid() {
    std::ifstream pid_max_file("/proc/sys/kernel/pid_max");
    int max_pid = 32768;
    if (pid_max_file) {
        pid_max_file >> max_pid;
    }
    return max_pid;
}

/// Get the cgroup v2 filesystem path of a process (Linux only)
/// @param pid Process ID to query
/// @return Cgroup path (empty string if not found or on cgroup v1)
inline std::string get_cgroup_v2_path(pid_t pid) {
    std::ifstream cgroup_file("/proc/" + std::to_string(pid) + "/cgroup");
    if (!cgroup_file) return "";
    
    std::string line;
    while (std::getline(cgroup_file, line)) {
        if (line.rfind("0::/", 0) == 0) {
            return "/sys/fs/cgroup" + line.substr(3);
        }
    }
    return "";
}
#endif

/// Check if a process still exists
/// @param pid Process ID to check
/// @return true if process exists
inline bool process_exists(pid_t pid) {
#ifdef _WIN32
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (hProcess) {
        CloseHandle(hProcess);
        return true;
    }
    return false;
#else
    return kill(pid, 0) == 0 || errno == EPERM;
#endif
}

/// Check if the current user can control a process
/// @param pid Process ID to check
/// @return true if process exists and is accessible
inline bool can_control_process(pid_t pid) {
#ifdef _WIN32
    HANDLE hProcess = OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, pid);
    if (hProcess) {
        CloseHandle(hProcess);
        return true;
    }
    return false;
#else
    if (kill(pid, 0) == 0) {
        return true;
    }
    return errno == EPERM;
#endif
}

/// Find the first process ID by executable name
/// @param exe_name Name of the executable (e.g., "notepad.exe" on Windows, "firefox" on Linux)
/// @return PID if found, -1 otherwise
inline pid_t find_process_by_name(const std::string& exe_name) {
#ifdef _WIN32
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return static_cast<pid_t>(-1);
    }
    
    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32W);
    
    if (Process32FirstW(hSnapshot, &pe32)) {
        do {
            // Convert wide string to narrow string for comparison
            char szProcessName[MAX_PATH];
            WideCharToMultiByte(CP_UTF8, 0, pe32.szExeFile, -1, 
                              szProcessName, MAX_PATH, nullptr, nullptr);
            
            if (exe_name == szProcessName) {
                CloseHandle(hSnapshot);
                return pe32.th32ProcessID;
            }
        } while (Process32NextW(hSnapshot, &pe32));
    }
    
    CloseHandle(hSnapshot);
    return static_cast<pid_t>(-1);
#else
    DIR* proc_dir = opendir("/proc");
    if (!proc_dir) {
        perror("[procctrl] Failed to open /proc");
        return -1;
    }
    
    struct dirent* entry;
    pid_t result = -1;
    
    while ((entry = readdir(proc_dir)) != nullptr) {
        // Skip non-directories
        if (entry->d_type != DT_UNKNOWN && entry->d_type != DT_DIR) continue;
        
        char* endptr;
        pid_t pid = strtol(entry->d_name, &endptr, 10);
        if (*endptr != '\0') continue;
        
        std::ifstream comm("/proc/" + std::string(entry->d_name) + "/comm");
        if (!comm) continue;
        
        std::string pname;
        std::getline(comm, pname);
        if (pname == exe_name) {
            result = pid;
            break;
        }
    }
    
    closedir(proc_dir);
    return result;
#endif
}

/// Find all process IDs by executable name
/// @param exe_name Name of the executable
/// @return Vector of PIDs (empty if none found)
inline std::vector<pid_t> find_all_processes_by_name(const std::string& exe_name) {
    std::vector<pid_t> pids;
    
#ifdef _WIN32
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return pids;
    }
    
    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32W);
    
    if (Process32FirstW(hSnapshot, &pe32)) {
        do {
            char szProcessName[MAX_PATH];
            WideCharToMultiByte(CP_UTF8, 0, pe32.szExeFile, -1, 
                              szProcessName, MAX_PATH, nullptr, nullptr);
            
            if (exe_name == szProcessName) {
                pids.push_back(pe32.th32ProcessID);
            }
        } while (Process32NextW(hSnapshot, &pe32));
    }
    
    CloseHandle(hSnapshot);
#else
    DIR* proc_dir = opendir("/proc");
    if (!proc_dir) {
        perror("[procctrl] Failed to open /proc");
        return pids;
    }
    
    struct dirent* entry;
    while ((entry = readdir(proc_dir)) != nullptr) {
        // Skip non-directories
        if (entry->d_type != DT_UNKNOWN && entry->d_type != DT_DIR) continue;
        
        char* endptr;
        pid_t pid = strtol(entry->d_name, &endptr, 10);
        if (*endptr != '\0') continue;
        
        std::ifstream comm("/proc/" + std::string(entry->d_name) + "/comm");
        if (!comm) continue;
        
        std::string pname;
        std::getline(comm, pname);
        if (pname == exe_name) {
            pids.push_back(pid);
        }
    }
    
    closedir(proc_dir);
#endif
    
    return pids;
}

/// Get the parent process ID (PPID) of a given process
/// @param pid Process ID to query
/// @return Parent PID if successful, -1 on error
inline pid_t get_parent_pid(pid_t pid) {
#ifdef _WIN32
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return static_cast<pid_t>(-1);
    }
    
    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32W);
    
    pid_t ppid = static_cast<pid_t>(-1);
    if (Process32FirstW(hSnapshot, &pe32)) {
        do {
            if (pe32.th32ProcessID == pid) {
                ppid = pe32.th32ParentProcessID;
                break;
            }
        } while (Process32NextW(hSnapshot, &pe32));
    }
    
    CloseHandle(hSnapshot);
    return ppid;
#else
    std::ifstream stat_file("/proc/" + std::to_string(pid) + "/stat");
    if (!stat_file) return -1;
    
    std::string line;
    std::getline(stat_file, line);
    
    size_t pos = line.rfind(')');
    if (pos == std::string::npos) return -1;
    
    std::istringstream iss(line.substr(pos + 1));
    char state;
    pid_t ppid;
    if (iss >> state >> ppid) {
        return ppid;
    }
    
    return -1;
#endif
}

/// Suspend or resume a process
/// @param pid Process ID to control
/// @param suspend true to suspend, false to resume
/// @return true if successful, false on error
inline bool set_process_suspended(pid_t pid, bool suspend) {
    if (!process_exists(pid)) {
#ifdef _WIN32
        fprintf(stderr, "[procctrl] PID %lu no longer exists\n", static_cast<unsigned long>(pid));
#else
        fprintf(stderr, "[procctrl] PID %d no longer exists\n", static_cast<int>(pid));
#endif
        return false;
    }
    
#ifdef _WIN32
    init_nt_functions();
    
    if (!g_pfnNtSuspendProcess || !g_pfnNtResumeProcess) {
        fprintf(stderr, "[procctrl] Failed to load NT functions\n");
        return false;
    }
    
    HANDLE hProcess = OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, pid);
    if (!hProcess) {
        fprintf(stderr, "[procctrl] Failed to open process %lu: error %lu\n", 
                static_cast<unsigned long>(pid), GetLastError());
        return false;
    }
    
    LONG status;
    if (suspend) {
        status = g_pfnNtSuspendProcess(hProcess);
        printf("[procctrl] Suspended PID %lu\n", static_cast<unsigned long>(pid));
    } else {
        status = g_pfnNtResumeProcess(hProcess);
        printf("[procctrl] Resumed PID %lu\n", static_cast<unsigned long>(pid));
    }
    
    CloseHandle(hProcess);
    return status == 0;
#else
    std::string cgroup_path = get_cgroup_v2_path(pid);
    const char* action_signal = suspend ? "SIGSTOP" : "SIGCONT";
    const char* action_cgroup = suspend ? "Freezing" : "Thawing";
    int signal_to_send = suspend ? SIGSTOP : SIGCONT;

    bool is_sandboxed_app = !cgroup_path.empty() &&
                            (cgroup_path.find("app-") != std::string::npos ||
                             cgroup_path.find("snap.") != std::string::npos);

    if (is_sandboxed_app && is_cgroup_v2_available()) {
        printf("[procctrl] PID %d belongs to sandboxed app. %s cgroup: %s\n",
               static_cast<int>(pid), action_cgroup, cgroup_path.c_str());
        
        std::string freeze_file_path = cgroup_path + "/cgroup.freeze";
        std::ofstream freeze_file(freeze_file_path);
        
        if (freeze_file) {
            freeze_file << (suspend ? "1" : "0");
            if (freeze_file.fail()) {
                fprintf(stderr, "[procctrl] Failed to write to %s: %s\n",
                        freeze_file_path.c_str(), strerror(errno));
                return false;
            }
            return true;
        } else {
            fprintf(stderr, "[procctrl] Failed to open %s: %s\n",
                    freeze_file_path.c_str(), strerror(errno));
            return false;
        }
    } else {
        printf("[procctrl] PID %d not sandboxed. Sending %s.\n",
               static_cast<int>(pid), action_signal);
        
        if (kill(pid, signal_to_send) != 0) {
            fprintf(stderr, "[procctrl] Error sending %s to PID %d: %s\n",
                    action_signal, static_cast<int>(pid), strerror(errno));
            return false;
        }
        return true;
    }
#endif
}

/// Suspend all processes by executable name (each cgroup only once on Linux)
/// @param exe_name Name of the executable
/// @return Number of processes/cgroups successfully suspended
inline int suspend_processes_by_name(const std::string& exe_name) {
#ifdef _WIN32
    int success_count = 0;
    for (auto pid : find_all_processes_by_name(exe_name)) {
        if (set_process_suspended(pid, true)) {
            success_count++;
        }
    }
    return success_count;
#else
    std::unordered_set<std::string> handled_cgroups;
    int success_count = 0;
    
    for (auto pid : find_all_processes_by_name(exe_name)) {
        std::string cgroup_path = get_cgroup_v2_path(pid);
        if (!cgroup_path.empty()) {
            if (handled_cgroups.count(cgroup_path)) continue;
            handled_cgroups.insert(cgroup_path);
        }
        
        if (set_process_suspended(pid, true)) {
            success_count++;
        }
    }
    
    return success_count;
#endif
}

/// Resume all processes by executable name (each cgroup only once on Linux)
/// @param exe_name Name of the executable
/// @return Number of processes/cgroups successfully resumed
inline int resume_processes_by_name(const std::string& exe_name) {
#ifdef _WIN32
    int success_count = 0;
    for (auto pid : find_all_processes_by_name(exe_name)) {
        if (set_process_suspended(pid, false)) {
            success_count++;
        }
    }
    return success_count;
#else
    std::unordered_set<std::string> handled_cgroups;
    int success_count = 0;
    
    for (auto pid : find_all_processes_by_name(exe_name)) {
        std::string cgroup_path = get_cgroup_v2_path(pid);
        if (!cgroup_path.empty()) {
            if (handled_cgroups.count(cgroup_path)) continue;
            handled_cgroups.insert(cgroup_path);
        }
        
        if (set_process_suspended(pid, false)) {
            success_count++;
        }
    }
    
    return success_count;
#endif
}

/// Get all PIDs in a process tree (parent and all descendants)
/// @param root_pid Root process ID
/// @return Vector of all PIDs in the tree including root_pid
inline std::vector<pid_t> get_process_tree(pid_t root_pid) {
    std::vector<pid_t> tree = {root_pid};
    std::vector<pid_t> to_check = {root_pid};
    
    while (!to_check.empty()) {
        pid_t current = to_check.back();
        to_check.pop_back();
        
#ifdef _WIN32
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnapshot == INVALID_HANDLE_VALUE) continue;
        
        PROCESSENTRY32W pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32W);
        
        if (Process32FirstW(hSnapshot, &pe32)) {
            do {
                if (pe32.th32ParentProcessID == current) {
                    tree.push_back(pe32.th32ProcessID);
                    to_check.push_back(pe32.th32ProcessID);
                }
            } while (Process32NextW(hSnapshot, &pe32));
        }
        
        CloseHandle(hSnapshot);
#else
        DIR* proc_dir = opendir("/proc");
        if (!proc_dir) continue;
        
        struct dirent* entry;
        while ((entry = readdir(proc_dir)) != nullptr) {
            // Skip non-directories
            if (entry->d_type != DT_UNKNOWN && entry->d_type != DT_DIR) continue;
            
            char* endptr;
            pid_t pid = strtol(entry->d_name, &endptr, 10);
            if (*endptr != '\0') continue;
            
            if (get_parent_pid(pid) == current) {
                tree.push_back(pid);
                to_check.push_back(pid);
            }
        }
        closedir(proc_dir);
#endif
    }
    
    return tree;
}

} // namespace procctrl