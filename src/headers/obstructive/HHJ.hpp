#pragma once
#include "Globals.hpp"
#include "Helper.hpp"
#include <thread>
#include <atomic>
#include <chrono>

// HHJ-specific speedglitch loop (runs in separate thread)
inline void hhjSpeedglitchLoop()
{
    int sleep1 = 16, sleep2 = 16;
    int last_fps = 0;
    const float EPSILON = 0.008f;

    while (true) {
        // Wait until HHJ speedglitch is activated
        while (!hhj_speedglitch_active.load(std::memory_order_relaxed)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        // Recalculate delays if FPS changed
        if (last_fps != roblox_fps) {
            float delay_float = 1000.0f / static_cast<float>(roblox_fps);
            int delay_floor = static_cast<int>(delay_float);
            int delay_ceil = delay_floor + 1;
            float fractional = delay_float - delay_floor;

            if (fractional < 0.33f - EPSILON)
            {
                sleep1 = sleep2 = delay_floor;
            }
            else if (fractional > 0.66f + EPSILON)
            {
                sleep1 = sleep2 = delay_ceil;
            }
            else
            {
                sleep1 = delay_floor;
                sleep2 = delay_ceil;
            }

            last_fps = roblox_fps;
        }

        // Perform the speedglitch rotation
        input.moveMouse(speed_pixels_x, 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep1));
        input.moveMouse(speed_pixels_y, 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep2));
    }
}

// Initialize HHJ system
inline void initHHJ()
{
    hhj_speedglitch_thread = std::thread(hhjSpeedglitchLoop);
    hhj_speedglitch_thread.detach();
    log("HHJ system initialized");
}

// Main HHJ Macro Function
inline void helicopterHighJump()
{
    static bool last_key_state = false;
    bool key_pressed = input.isKeyPressed(Binds["HHJ"]);

    // Trigger on key press (not hold)
    if (key_pressed && !last_key_state)
    {
        log("HHJ triggered");

        // AUTO-TIMING MODE (Experimental)
        if (hhj_auto_timing)
        {
            input.holdKey(CrossInput::Key::Space); // Jump
            std::this_thread::sleep_for(std::chrono::milliseconds(550));
            input.holdKey(CrossInput::Key::W); // Hold W
            std::this_thread::sleep_for(std::chrono::milliseconds(68));
        }

        // FREEZE PHASE
        procctrl::suspend_processes_by_name(roblox_process_name);
        log("HHJ: Game suspended");

        // Determine freeze duration
        int freeze_duration = 200; // Base duration

        if (hhj_freeze_delay > 0)
        {
            // User override
            freeze_duration = hhj_freeze_delay;
        }
        else
        {
            // Default: 500ms total, or 200ms if fast mode
            if (!hhj_fast_mode)
            {
                freeze_duration += 300; // 500ms total
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(freeze_duration));

        // Release auto-timing keys if active
        if (hhj_auto_timing)
        {
            input.releaseKey(CrossInput::Key::Space);
            input.releaseKey(CrossInput::Key::W);
        }

        // UNFREEZE PHASE
        procctrl::resume_processes_by_name(roblox_process_name);
        log("HHJ: Game resumed");

        // Delay 1: Wait before shiftlock
        std::this_thread::sleep_for(std::chrono::milliseconds(hhj_delay1));

        // Hold shiftlock (or zoom in if configured)
        if (!globalzoomin)
        {
            input.holdKey(CrossInput::Key::LShift);
        }
        else
        {
            // Note: Mouse wheel simulation would go here
            // For now using shift as fallback
            input.holdKey(CrossInput::Key::LShift);
        }

        // Delay 2: Wait before spinning
        std::this_thread::sleep_for(std::chrono::milliseconds(hhj_delay2));

        // START SPINNING (activate HHJ speedglitch)
        hhj_speedglitch_active.store(true, std::memory_order_relaxed);
        log("HHJ: Spinning started");

        // Delay 3: Hold shiftlock while spinning
        std::this_thread::sleep_for(std::chrono::milliseconds(hhj_delay3));

        // Release shiftlock
        if (!globalzoomin)
        {
            input.releaseKey(CrossInput::Key::LShift);
        }

        // Continue spinning for HHJ length
        std::this_thread::sleep_for(std::chrono::milliseconds(hhj_length));

        // STOP SPINNING
        hhj_speedglitch_active.store(false, std::memory_order_relaxed);
        log("HHJ: Spinning stopped");

        log("HHJ completed");
    }

    last_key_state = key_pressed;
}

// Update HHJ settings functions
inline void updateHHJLength(int new_length)
{
    hhj_length = new_length;
    log("HHJ length updated to: " + std::to_string(new_length) + "ms");
}

inline void updateHHJDelays(int delay1, int delay2, int delay3)
{
    hhj_delay1 = delay1;
    hhj_delay2 = delay2;
    hhj_delay3 = delay3;
    log("HHJ delays updated: " + std::to_string(delay1) + ", " +
        std::to_string(delay2) + ", " + std::to_string(delay3));
}

inline void updateHHJFreezeDelay(int delay)
{
    hhj_freeze_delay = delay;
    log("HHJ freeze delay override: " + std::to_string(delay) + "ms");
}

inline void setHHJAutoTiming(bool enabled)
{
    hhj_auto_timing = enabled;
    log("HHJ auto-timing: " + std::string(enabled ? "enabled" : "disabled"));
}

inline void setHHJFastMode(bool enabled)
{
    hhj_fast_mode = enabled;
    log("HHJ fast mode: " + std::string(enabled ? "enabled" : "disabled"));
}