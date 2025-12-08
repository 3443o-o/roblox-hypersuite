#pragma once
#include <cstdlib>
#include <sstream>
#include "Globals.hpp"
#include "inpctrl.hpp"
#include "Helper.hpp"
#include "netctrl.hpp"

inline namespace LagSwitchNamespace {
    inline bool TrafficBlocked = false;
    inline bool PreventDisconnection = true;
    inline int LagTimeMilliseconds = 1;
    inline float PacketLossPercentage = 99.5f;
    inline bool customValuesAllowed = false;

    // New: Ping increase mode
    inline bool PingIncreaseMode = false;  // Toggle between lag switch and ping increase
    inline int PingIncreaseAmount = 100;   // Default 100ms ping increase

    // Note: Using global ctrl from Globals.hpp (no duplicate declaration)

    inline bool BlockTraffic() {
        if (TrafficBlocked) {
            log("Traffic already blocked");
            return true;
        }

        bool success = false;

        // Check if we're in ping increase mode
        if (PingIncreaseMode) {
            log("Increasing ping by " + std::to_string(PingIncreaseAmount) + "ms");
            success = ctrl.increasePing(PingIncreaseAmount);

            if (success) {
                log("[NetCtrl] Ping increased successfully");
                TrafficBlocked = true;
            } else {
                log("[NetCtrl] Failed to increase ping");
            }
            return success;
        }

        // Regular lag switch mode
        log("Blocking outbound traffic for " + roblox_process_name);

        int lag_ms = customValuesAllowed ? LagTimeMilliseconds : 1;
        float drop_pct = customValuesAllowed ? PacketLossPercentage : 99.5f;

        if (PreventDisconnection) {
            // Apply lag + packet loss to prevent disconnection
            log("[NetCtrl] Preventing disconnection: lag=" + std::to_string(lag_ms) +
                "ms, drop=" + std::to_string(drop_pct) + "%");

            success = ctrl.lag(lag_ms, static_cast<double>(drop_pct));

            if (success) {
                log("[NetCtrl] Successfully applied lag/drop");
                TrafficBlocked = true;
            } else {
                log("[NetCtrl] Failed to apply lag/drop");
            }
        } else {
            // Full block (100% packet loss)
            log("[NetCtrl] Blocking all traffic (100% drop)");
            success = ctrl.block();

            if (success) {
                log("[NetCtrl] Successfully blocked traffic");
                TrafficBlocked = true;
            } else {
                log("[NetCtrl] Failed to block traffic");
            }
        }

        return success;
    }

    inline bool UnblockTraffic() {
        if (!TrafficBlocked) {
            log("Traffic already unblocked");
            return true;
        }

        log("Unblocking outbound traffic for " + roblox_process_name);
        log("[NetCtrl] Disabling all network controls...");

        bool success = ctrl.disable();

        if (success) {
            TrafficBlocked = false;
            log("[NetCtrl] Successfully unblocked traffic");
        } else {
            log("[NetCtrl] Failed to disable network controls");
        }

        return success;
    }
}

inline void LagSwitch() {
    bool key_pressed = input.isKeyPressed(Binds["Lag-switch"]);

    if (!key_pressed && events[4]) {
        if (LagSwitchNamespace::TrafficBlocked) {
            LagSwitchNamespace::UnblockTraffic();
        } else {
            LagSwitchNamespace::BlockTraffic();
        }
    }
    events[4] = key_pressed;
}
