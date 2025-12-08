#pragma once
#include "Globals.hpp"
#include "Helper.hpp"
#include "procctrl.hpp"

// Gear Desync Configuration
inline int desync_gear_slot = 1;
inline std::atomic<bool> desync_active(false);
inline std::thread desync_thread;

// Gear desync loop (runs in separate thread)
inline void gearDesyncLoop() {
    while (true) {
        // Wait until desync is activated
        while (!desync_active.load(std::memory_order_relaxed)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        
        // Rapidly spam the gear slot
        // This needs to be VERY fast to trigger server throttling
        input.pressKey(static_cast<CrossInput::Key>(
            static_cast<int>(CrossInput::Key::Num0) + desync_gear_slot), 0);
        
        // No delay - we want this as fast as possible
        // The loop will naturally have some tiny delay from the function calls
    }
}


// Initialize gear desync system
inline void initGearDesync() {
    desync_thread = std::thread(gearDesyncLoop);
    desync_thread.detach();
    log("Gear Desync system initialized");
}

// Main gear desync macro
inline void gearDesyncMacro() {
    bool key_pressed = input.isKeyPressed(Binds["Gear-Desync"]);
    
    // This is a HOLD macro - must hold key to desync
    // User needs to hold for 4-7 seconds
    if (key_pressed && !events[8]) {
        events[8] = true;
        desync_active.store(true, std::memory_order_relaxed);
        log("Gear Desync activated - hold for 4-7 seconds");
    }
    
    if (!key_pressed && events[8]) {
        events[8] = false;
        desync_active.store(false, std::memory_order_relaxed);
        log("Gear Desync deactivated");
    }
}

// Update gear slot
inline void updateDesyncGearSlot(int new_slot) {
    // Clamp to valid range (1-9, 0 for slot 10)
    if (new_slot < 0) new_slot = 0;
    if (new_slot > 9) new_slot = 9;

    desync_gear_slot = new_slot;
    log("Gear Desync slot updated to: " + std::to_string(new_slot));
}