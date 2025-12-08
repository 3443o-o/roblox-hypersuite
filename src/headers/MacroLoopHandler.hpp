#pragma once
#include "Globals.hpp"
#include "Macros.hpp"
#include "LagSwitch.hpp"
#include "Speedglitch.hpp"
#include "HHJ.hpp"
#include "GearDesync.hpp"

inline void initMacros() {
    initSpeedglitch();
    initHHJ();
    initGearDesync();
}

inline void UpdateMacros() {
    if (enabled[0]) freezeMacro();
    if (enabled[1]) laughClip();
    if (enabled[2]) extendedDanceClip();
    if (enabled[4]) LagSwitch();
    if (enabled[5]) BuckeyClip();
    if (enabled[6]) speedglitchMacro();
    if (enabled[7]) SpamKeyMacro();
    if (enabled[10]) DisableHeadCollision();
    if (enabled[11]) NHCRoofClip();
    if (enabled[12]) helicopterHighJump();
    //if (enabled[13]) gearDesyncMacro();
    if (enabled[14]) FullGearDesync();
}