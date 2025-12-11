#pragma once
#include "Globals.hpp"
#include "Helper.hpp"

inline void freezeMacro() {
    bool key_pressed = input.isKeyPressed(Binds["Freeze"]);

    if (key_pressed && !events[0]) {
        log("Freeze triggered for " + roblox_process_name);
        procctrl::suspend_processes_by_name(roblox_process_name);
    }

    if (!key_pressed && events[0]) {
        log("Unfreeze triggered for " + roblox_process_name);
        procctrl::resume_processes_by_name(roblox_process_name);
    }
    events[0] = key_pressed;
}

inline void laughClip() {
    bool key_pressed = input.isKeyPressed(Binds["Laugh"]);
    if (key_pressed && !events[1]) {
        events[1] = true;
        log("Laugh clip triggered");

        input.pressKey(ChatKey);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        if (kb_layout == 1) {
            typeSlashAzerty();
            input.typeText("e lqugh");
        } else {
            input.typeText("/e laugh");
        }
        input.pressKey(CrossInput::Key::Enter);

        std::this_thread::sleep_for(std::chrono::milliseconds(248));
        input.holdKey(CrossInput::Key::S);
        procctrl::suspend_processes_by_name(roblox_process_name);

        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        procctrl::resume_processes_by_name(roblox_process_name);

        input.holdKey(CrossInput::Key::Space);
        input.holdKey(CrossInput::Key::LShift);

        std::this_thread::sleep_for(std::chrono::milliseconds(35));

        input.releaseKey(CrossInput::Key::Space);
        input.releaseKey(CrossInput::Key::LShift);

        std::this_thread::sleep_for(std::chrono::milliseconds(40));

        input.releaseKey(CrossInput::Key::S);

        events[1] = false;
        log("Laugh clip finished");
    }
}

inline void extendedDanceClip() {
    bool key_pressed = input.isKeyPressed(Binds["E-Dance"]);
    if (key_pressed && !events[2]) {
        events[2] = true;
        log("Extended Dance clip triggered");

        input.pressKey(ChatKey);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        if (kb_layout == 1) {
            typeSlashAzerty();
            input.typeText("e dqnce");
            input.holdKey(CrossInput::Key::LShift);
            input.pressKey(CrossInput::Key::Num2);
            input.releaseKey(CrossInput::Key::LShift);
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
            input.pressKey(CrossInput::Key::Enter);
        } else {
            input.typeText("/e dance2");
            input.pressKey(CrossInput::Key::Enter);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(815));

        input.holdKey(CrossInput::Key::D);
        procctrl::suspend_processes_by_name(roblox_process_name);

        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        procctrl::resume_processes_by_name(roblox_process_name);

        input.pressKey(CrossInput::Key::LShift);

        std::this_thread::sleep_for(std::chrono::milliseconds(300));

        input.releaseKey(CrossInput::Key::D);

        events[2] = false;
        log("Extended Dance clip finished");
    }
}

inline void BuckeyClip() {
    bool key_pressed = input.isKeyPressed(Binds["Buckey-clip"]);
    if (key_pressed && !events[5]) {
        events[5] = true;
        log("Buckey clip triggered");

        input.pressKey(ChatKey);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        if (kb_layout == 1) {
            typeSlashAzerty();
            input.typeText("e lqugh");
        } else {
            input.typeText("/e laugh");
        }

        input.pressKey(CrossInput::Key::Enter);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        // --- Space key ---
        input.holdKey(CrossInput::Key::Space);

        // Wait 31ms → S down while Space is still held
        std::this_thread::sleep_for(std::chrono::milliseconds(31));
        input.holdKey(CrossInput::Key::S);

        // Wait 9ms → Shift down while Space+S are still held
        std::this_thread::sleep_for(std::chrono::milliseconds(9));
        input.holdKey(CrossInput::Key::LShift);

        // Release Space after 74ms
        std::this_thread::sleep_for(std::chrono::milliseconds(74));
        input.releaseKey(CrossInput::Key::Space);

        // Release Shift after 56ms
        std::this_thread::sleep_for(std::chrono::milliseconds(56));
        input.releaseKey(CrossInput::Key::LShift);

        // Release S after 82ms
        std::this_thread::sleep_for(std::chrono::milliseconds(82));
        input.releaseKey(CrossInput::Key::S);

        events[5] = false;
        log("Buckeyclip finished");
    }
}


inline void SpamKeyMacro() {
    bool key_pressed = input.isKeyPressed(Binds["Spam-Key"]);
    if (key_pressed && !events[7]) {
        events[7] = true;
        log("Spam key triggering");
        input.pressKey(SpamKey, 1);
        events[7] = false;
        log("Spam key finished triggering");
    }
}

inline void DisableHeadCollision() {
    bool key_pressed = input.isKeyPressed(Binds["Disable-Head-Collision"]);
    if (key_pressed && !events[10]) {
        events[10] = true;
        log("Disable-Head-Collision triggered");

        input.pressKey(ChatKey);
        std::this_thread::sleep_for(std::chrono::milliseconds(248));

        if (kb_layout == 1) {
            typeSlashAzerty();
            input.typeText("e lqugh");
        } else {
            input.typeText("/e laugh");
        }

        input.pressKey(CrossInput::Key::Enter);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        input.holdKey(CrossInput::Key::LShift);
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
        input.releaseKey(CrossInput::Key::LShift);

        input.holdKey(CrossInput::Key::Space);
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
        input.releaseKey(CrossInput::Key::Space);

        input.holdKey(CrossInput::Key::LShift);
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
        input.releaseKey(CrossInput::Key::LShift);

        events[10] = false;
        log("Disable-Head-Collision finished");
    }
}

inline void NHCRoofClip() {
    bool key_pressed = input.isKeyPressed(Binds["NHC-Roof"]);
    if (key_pressed && !events[11]) {
        events[11] = true;
        log("NHC-Roof clip triggered");

        input.pressKey(ChatKey);
        std::this_thread::sleep_for(std::chrono::milliseconds(248));

        if (kb_layout == 1) {
            typeSlashAzerty();
            input.typeText("e cheer", 20);
        } else {
            input.typeText("/e cheer", 20);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        input.pressKey(CrossInput::Key::Enter);
        std::this_thread::sleep_for(std::chrono::milliseconds(610));

        input.holdKey(CrossInput::Key::Space);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        procctrl::suspend_processes_by_name(roblox_process_name);
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        procctrl::resume_processes_by_name(roblox_process_name);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        input.releaseKey(CrossInput::Key::Space);

        events[11] = false;
        log("NHC-Roof clip finished");
    }
}


inline void FullGearDesync() {
    bool key_pressed = input.isKeyPressed(Binds["Full-Gear-Desync"]);
    if (key_pressed && !events[14]) {
        events[14] = true;
        log("Full Gear Desync triggered");
        input.pressKey(CrossInput::Key::Num2);
        std::this_thread::sleep_for(std::chrono::milliseconds(40));
        input.pressKey(CrossInput::Key::Backspace);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        input.pressKey(CrossInput::Key::Num1);
        std::this_thread::sleep_for(std::chrono::milliseconds(40));
        input.holdKey(CrossInput::Key::W);
        std::this_thread::sleep_for(std::chrono::milliseconds(400));
        procctrl::suspend_processes_by_name(roblox_process_name);
        input.pressKey(CrossInput::Key::Num1, 30);
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
        input.pressKey(CrossInput::Key::Num1, 30);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        procctrl::resume_processes_by_name(roblox_process_name);
        input.releaseKey(CrossInput::Key::W);
        std::this_thread::sleep_for(std::chrono::milliseconds(40));
        input.pressKey(CrossInput::Key::Num1);
        std::this_thread::sleep_for(std::chrono::milliseconds(40));
        input.pressKey(CrossInput::Key::Backspace);
        events[14] = false;
        log("Full Gear Desync finished");
    }
}

inline void FloorBounceHighJump() {
    bool key_pressed = input.isKeyPressed(Binds["Floor-Bounce-High-Jump"]);
    if (key_pressed && !events[15]) {
        events[15] = true;
        log("Floor bounce high jump triggered");

        input.holdKey(CrossInput::Key::Space);
        std::this_thread::sleep_for(std::chrono::milliseconds(521));  // Fall timing
        procctrl::suspend_processes_by_name(roblox_process_name);     // Freeze to clip through floor
        std::this_thread::sleep_for(std::chrono::milliseconds(72));   // Stay clipped
        procctrl::resume_processes_by_name(roblox_process_name);      // Register underground position
        std::this_thread::sleep_for(std::chrono::milliseconds(72));   // Let correction force build
        procctrl::suspend_processes_by_name(roblox_process_name);     // Freeze the ejection force
        std::this_thread::sleep_for(std::chrono::milliseconds(72));   // Hold the power
        procctrl::resume_processes_by_name(roblox_process_name);      // LAUNCH
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        input.releaseKey(CrossInput::Key::Space);

        events[15] = false;
        log("Floor bounce high jump finished");
    }
}
