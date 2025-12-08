/*
===============================================================================
NetCtrl - Cross-Platform Network Control Library (Header-Only)
===============================================================================

Author: 3443
Date: [22/11/2025]
License: MIT

Description:
-------------
NetCtrl is a lightweight, header-only C++ library for programmatically
controlling network traffic. Uses Fumble on Windows and tc netem on Linux.

Features:
----------
- Block network traffic completely
- Increase ping/latency (RTT)
- Apply packet loss
- Combine ping increase + packet loss
- Clean removal of all rules
- Header-only, cross-platform

Windows Requirements:
---------------------
- fumble.exe must be in resources/ folder or system PATH
- Requires administrator privileges
- Fumble: https://github.com/zp4rand0miz31/fumble

Linux Requirements:
-------------------
- tc (traffic control) utility (iproute2 package)
- iptables
- Requires root privileges

Usage Example (C++):
---------------------
#include "netctrl.hpp"
#include <iostream>

int main() {
    netctrl::NetCtrl net;

    if (!netctrl::NetCtrl::isAdmin()) {
        std::cerr << "Run as administrator/root!" << std::endl;
        return 1;
    }

    // Increase ping by 200ms
    net.increasePing(200);

    // Or combine ping increase + packet loss
    net.lag(150, 5.0);  // 150ms delay + 5% packet loss

    // Block all traffic
    net.block();

    // Remove all controls
    net.disable();

    return 0;
}

===============================================================================
*/

#ifndef NETCTRL_HPP
#define NETCTRL_HPP

#include <string>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <iomanip>
#include <iostream>

#ifdef _WIN32
#ifndef NETCTRL_WINDOWS_INCLUDED
#define NETCTRL_WINDOWS_INCLUDED
#define WIN32_LEAN_AND_MEAN

#ifndef NOMINMAX
#define NOMINMAX
#endif

// Rename Windows functions to avoid conflicts with Raylib
#define Rectangle Win32Rectangle
#define CloseWindow Win32CloseWindow
#define ShowCursor Win32ShowCursor
#define DrawText Win32DrawText
#define DrawTextEx Win32DrawTextEx
#define LoadImage Win32LoadImage

#include <windows.h>
#include <tlhelp32.h>

// Restore original names after Windows header is included
#undef Rectangle
#undef CloseWindow
#undef ShowCursor
#undef DrawText
#undef DrawTextEx
#undef LoadImage

#endif
#else
#include <unistd.h>
#endif

namespace netctrl {

enum class Direction {
    Inbound,
    Outbound,
    Both
};

class NetCtrl {
public:
    NetCtrl() {
        findInterface();
#ifdef _WIN32
        initFumbleJob();
#endif
    }

    ~NetCtrl() {
        disable();
#ifdef _WIN32
        cleanupFumble();
#endif
    }

    // ========================================================================
    // PRIMARY API METHODS
    // ========================================================================

    /**
     * Increase ping/latency by specified milliseconds
     * @param ms Milliseconds to add to network latency (RTT will increase by ~2x this)
     * @return true if successful
     */
    bool increasePing(int ms) {
        if (ms <= 0) {
            std::cerr << "[ERROR] Ping increase must be > 0" << std::endl;
            return false;
        }
        std::cout << "[INFO] Increasing ping by " << ms << "ms" << std::endl;
        return lag(ms, 0.0);
    }

    /**
     * Apply network lag with optional packet loss
     * @param lag_ms Delay in milliseconds (increases ping/RTT)
     * @param drop_percent Packet loss percentage (0-100)
     * @return true if successful
     */
    bool lag(int lag_ms, double drop_percent) {
        std::cout << "[INFO] Applying lag=" << lag_ms << "ms, loss=" << drop_percent << "%" << std::endl;
#ifdef _WIN32
        return applyWindowsLag(lag_ms, drop_percent);
#else
        return applyLinux(lag_ms, drop_percent);
#endif
    }

    /**
     * Block all network traffic completely (100% packet loss)
     * @return true if successful
     */
    bool block() {
        std::cout << "[INFO] Blocking all network traffic..." << std::endl;
#ifdef _WIN32
        return applyWindowsLag(1, 100.0);
#else
        return blockLinux();
#endif
    }

    /**
     * Remove all network controls and restore normal operation
     * @return true if successful
     */
    bool disable() {
        std::cout << "[INFO] Disabling all network controls..." << std::endl;
#ifdef _WIN32
        return disableWindows();
#else
        return disableLinux();
#endif
    }

    // ========================================================================
    // STATUS METHODS
    // ========================================================================

    bool isActive() const { return is_active_; }
    int getLag() const { return current_lag_ms_; }
    double getDrop() const { return current_drop_percent_; }

    static bool isAdmin() {
#ifdef _WIN32
        BOOL admin = FALSE;
        PSID grp = NULL;
        SID_IDENTIFIER_AUTHORITY auth = SECURITY_NT_AUTHORITY;
        if (AllocateAndInitializeSid(&auth, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                      DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &grp)) {
            CheckTokenMembership(NULL, grp, &admin);
            FreeSid(grp);
        }
        return admin;
#else
        return geteuid() == 0;
#endif
    }

private:
    bool is_active_ = false;
    int current_lag_ms_ = 0;
    double current_drop_percent_ = 0.0;
    std::string default_iface_;

#ifdef _WIN32
    HANDLE fumble_job_ = NULL;
    PROCESS_INFORMATION fumble_process_{};
#endif

    void findInterface() {
#ifndef _WIN32
        FILE* p = popen("ip route show default | awk '/default/ {print $5}' | head -1", "r");
        if (p) {
            char buf[64];
            if (fgets(buf, sizeof(buf), p)) {
                default_iface_ = buf;
                if (!default_iface_.empty() && default_iface_.back() == '\n') {
                    default_iface_.pop_back();
                }
            }
            pclose(p);
        }

        if (default_iface_.empty()) {
            std::vector<std::string> common = {"eth0", "eno1", "enp0s3", "wlan0", "wlp2s0"};
            for (const auto& iface : common) {
                std::string check = "ip link show " + iface + " 2>/dev/null";
                if (system(check.c_str()) == 0) {
                    default_iface_ = iface;
                    break;
                }
            }
        }

        std::cout << "[DEBUG] Using interface: " << default_iface_ << std::endl;
#endif
    }

#ifdef _WIN32
    void initFumbleJob() {
        if (!fumble_job_) {
            fumble_job_ = CreateJobObjectA(NULL, NULL);
            if (fumble_job_) {
                JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = {};
                jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
                SetInformationJobObject(fumble_job_, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli));
                std::cout << "[DEBUG] Job object created - Fumble will auto-close with program" << std::endl;
            } else {
                std::cerr << "[WARNING] Failed to create job object. Fumble may persist after exit." << std::endl;
            }
        }
    }

    void cleanupFumble() {
        if (fumble_process_.hProcess) {
            std::cout << "[DEBUG] Terminating fumble process..." << std::endl;
            TerminateProcess(fumble_process_.hProcess, 0);
            CloseHandle(fumble_process_.hProcess);
            CloseHandle(fumble_process_.hThread);
            ZeroMemory(&fumble_process_, sizeof(fumble_process_));
        }
        if (fumble_job_) {
            CloseHandle(fumble_job_);
            fumble_job_ = NULL;
        }
    }

    bool applyWindowsLag(int lag_ms, double drop_percent) {
        // Kill any existing fumble process first
        disableWindows();

        // Validate parameters
        if (lag_ms < 0) lag_ms = 1;
        if (drop_percent < 0) drop_percent = 0;
        if (drop_percent > 100) drop_percent = 100;

        // Build fumble command line arguments
        double drop_prob = drop_percent / 100.0;
        std::stringstream ss;

        // Filter UDP traffic on high ports (typical for games like Roblox)
        ss << " --filter \"outbound and udp and udp.DstPort >= 49152\"";

        if (lag_ms > 0) {
            ss << " --delay-duration " << lag_ms;
        }

        if (drop_percent > 0) {
            ss << " --drop-probability " << std::fixed << std::setprecision(4) << drop_prob;
        }

        std::string params = ss.str();

        // Try multiple possible locations for fumble.exe
        std::vector<std::string> fumble_paths = {
            "resources\\fumble.exe",
            "fumble.exe",
            ".\\fumble.exe",
            "..\\resources\\fumble.exe"
        };

        std::string fumble_path;
        for (const auto& path : fumble_paths) {
            if (GetFileAttributesA(path.c_str()) != INVALID_FILE_ATTRIBUTES) {
                fumble_path = path;
                break;
            }
        }

        if (fumble_path.empty()) {
            std::cerr << "[ERROR] fumble.exe not found! Please place it in resources/ folder" << std::endl;
            std::cerr << "[INFO] Download from: https://github.com/zp4rand0miz31/fumble" << std::endl;
            return false;
        }

        // Launch fumble with elevated privileges
        SHELLEXECUTEINFOA sei = {};
        sei.cbSize = sizeof(sei);
        sei.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_NO_CONSOLE;
        sei.lpVerb = "runas";  // Request admin elevation
        sei.lpFile = fumble_path.c_str();
        sei.lpParameters = params.c_str();
        sei.nShow = SW_HIDE;

        std::cout << "[DEBUG] Launching fumble: " << fumble_path << params << std::endl;

        if (!ShellExecuteExA(&sei)) {
            DWORD error = GetLastError();
            if (error == ERROR_CANCELLED) {
                std::cerr << "[ERROR] User cancelled UAC prompt" << std::endl;
            } else {
                std::cerr << "[ERROR] Failed to launch fumble.exe. Error: " << error << std::endl;
            }
            return false;
        }

        if (!sei.hProcess) {
            std::cerr << "[ERROR] Failed to get fumble process handle" << std::endl;
            return false;
        }

        // Store process info
        fumble_process_.hProcess = sei.hProcess;
        fumble_process_.dwProcessId = GetProcessId(sei.hProcess);

        // Assign to job object for automatic cleanup
        if (fumble_job_) {
            if (!AssignProcessToJobObject(fumble_job_, fumble_process_.hProcess)) {
                std::cerr << "[WARNING] Failed to assign fumble to job object" << std::endl;
            }
        }

        is_active_ = true;
        current_lag_ms_ = lag_ms;
        current_drop_percent_ = drop_percent;

        std::cout << "[SUCCESS] Fumble launched successfully!" << std::endl;
        if (lag_ms > 0) {
            std::cout << "  → Delay: " << lag_ms << "ms" << std::endl;
        }
        if (drop_percent > 0) {
            std::cout << "  → Packet loss: " << drop_percent << "%" << std::endl;
        }

        return true;
    }

    bool disableWindows() {
        cleanupFumble();

        is_active_ = false;
        current_lag_ms_ = 0;
        current_drop_percent_ = 0.0;

        std::cout << "[SUCCESS] Network controls disabled" << std::endl;
        return true;
    }
#else
    bool blockLinux() {
        disableLinux();

        std::cout << "[DEBUG] Adding iptables DROP rules..." << std::endl;
        system("iptables -w -I OUTPUT 1 -j DROP &");
        system("iptables -w -I INPUT 1 -j DROP &");

        is_active_ = true;
        current_lag_ms_ = 0;
        current_drop_percent_ = 100.0;
        return true;
    }

    bool applyLinux(int lag_ms, double drop_percent) {
        disableLinux();

        if (default_iface_.empty()) {
            std::cerr << "[ERROR] No network interface found!" << std::endl;
            return false;
        }

        std::cout << "[DEBUG] Applying tc netem on " << default_iface_ << std::endl;

        std::stringstream cmd;
        cmd << "tc qdisc add dev " << default_iface_ << " root netem";

        if (lag_ms > 0) {
            cmd << " delay " << lag_ms << "ms";
        }

        if (drop_percent > 0 && drop_percent < 100) {
            cmd << " loss " << std::fixed << std::setprecision(2) << drop_percent << "%";
        }

        cmd << " 2>/dev/null &";

        std::cout << "[DEBUG] Executing: " << cmd.str() << std::endl;
        system(cmd.str().c_str());

        is_active_ = true;
        current_lag_ms_ = lag_ms;
        current_drop_percent_ = drop_percent;

        std::cout << "[SUCCESS] Network control applied!" << std::endl;
        if (lag_ms > 0) {
            std::cout << "  → Ping increased by ~" << lag_ms << "ms (RTT: ~" << (lag_ms * 2) << "ms)" << std::endl;
        }
        if (drop_percent > 0) {
            std::cout << "  → Packet loss: " << drop_percent << "%" << std::endl;
        }

        return true;
    }

    bool disableLinux() {
        if (!default_iface_.empty()) {
            std::string cmd = "tc qdisc del dev " + default_iface_ + " root 2>/dev/null &";
            system(cmd.c_str());
        }

        system("iptables -w -D OUTPUT -j DROP 2>/dev/null & iptables -w -D INPUT -j DROP 2>/dev/null &");

        is_active_ = false;
        current_lag_ms_ = 0;
        current_drop_percent_ = 0.0;
        return true;
    }
#endif
};

} // namespace netctrl
#endif
