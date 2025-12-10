#include "headers/obstructive/WebAPI.hpp"
#define Font RaylibFont   // temporarily rename Raylib's Font

#include "raylib.h"

#undef Font  // restore the macro

#ifdef __linux__
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#endif


#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
// Define these BEFORE including Globals.hpp
#define Rectangle Win32Rectangle
#define CloseWindow Win32CloseWindow
#define ShowCursor Win32ShowCursor
#define DrawText Win32DrawText
#define DrawTextEx Win32DrawTextEx
#define LoadImage Win32LoadImage

#include <windows.h>  // Include Windows headers here with renames active
#undef Rectangle
#undef CloseWindow
#undef ShowCursor
#undef DrawText
#undef DrawTextEx
#undef LoadImage
#endif

#include "Globals.hpp"
#include "procctrl.hpp"
#include "netctrl.hpp"
#include "logzz.hpp"
#include "LagSwitch.hpp"
#include "MacroLoopHandler.hpp"
#include "Helper.hpp"
#include "RobloxFiles.hpp"
#include "UserInterface.hpp"
#include "LoadTextures.hpp"
#include "SettingsHandler.hpp"
#include "GlobalBasicSettings.hpp"
#include "imgui.h"
#include "rlImGui.h"

#include <iostream>
#include <string>
#include <thread>
#include <cstdlib>
#include <cstdio>
#include <map>

int main() {
#if defined(__linux__)
    //Fix for unable to open display ":0" on wayland
    runXhostPlus();
#else
    SetConfigFlags(FLAG_WINDOW_HIGHDPI);
    //SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_UNAWARE_GDISCALED); // Fixes weird scaling issues.
#endif
    is_elevated = isElevated();
    if (is_elevated) {
        InitWindow(500, 400, "Roblox hypersuite");
    } else InitWindow(300, 150, "Roblox hypersuite");

    screen_width = GetScreenWidth();
    screen_height = GetScreenHeight();

    // Default values
#ifdef _WIN32
    roblox_process_name = "RobloxPlayerBeta.exe";
    Image icon = LoadImage("resources/logo.png");
    SetWindowIcon(icon);                           // Sets taskbar + title bar icon
    UnloadImage(icon);                             // Free image memory
    logzz::logs_folder_path = getRobloxAppDataDirectory() + "\\logs";
#else
    roblox_process_name = "sober";
    logzz::logs_folder_path = getRobloxAppDataDirectory() + "/logs";
#endif

    kb_layout = 0;

    SetTargetFPS(60);
    testrealok();
    //-------- LOADING THE FREAKING SETTINGS
    SettingsHandler::LoadSettings();
    //Initializes the user interface.
    initUI();
    LoadAllSprites();
    // For globalbasicsettings
    if (GlobalBasicSettingsFile == "empty") setGBSFileDirectory();

    //Initlializes the ctrl object for netctrl
    g_ctrl = &ctrl;

    //Initializes the input object.
    if (!input.init()) {
        std::cerr << "Failed to initialize input system!\n";
        return 1;
    }

    initMacros();
    // No window border for windows :p
#ifdef _WIN32
    if (!decorated_window) SetWindowState(FLAG_WINDOW_UNDECORATED);
#endif


    Vector2 dragOffset = {0};
    bool isDragging = false;

    while (!WindowShouldClose()) {
       UpdateMacros();
       logzz::loop_handle();

       if (resizable_window != lastResizable) {
            if (resizable_window)
                SetWindowState(FLAG_WINDOW_RESIZABLE);
            else
                ClearWindowState(FLAG_WINDOW_RESIZABLE);

            lastResizable = resizable_window;
        }

        if (decorated_window != lastDecorated) {
            if (decorated_window)
                ClearWindowState(FLAG_WINDOW_UNDECORATED);
            else
                SetWindowState(FLAG_WINDOW_UNDECORATED);

            lastDecorated = decorated_window;
        }

        if (windowOnTop != lastWindowOnTop) {
#ifdef _WIN32
                HWND hwnd = (HWND)GetWindowHandle(); // cast void* -> HWND
                if (windowOnTop)
                    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                else
                    SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
#elif defined(__linux__)
                Display* display = XOpenDisplay(nullptr);
                if (display) {
                    Window win = (Window)GetWindowHandle(); // Raylib window handle
                    Atom wm_state = XInternAtom(display, "_NET_WM_STATE", False);
                    Atom wm_above = XInternAtom(display, "_NET_WM_STATE_ABOVE", False);

                    XEvent xev;
                    memset(&xev, 0, sizeof(xev));
                    xev.xclient.type = ClientMessage;
                    xev.xclient.window = win;
                    xev.xclient.message_type = wm_state;
                    xev.xclient.format = 32;
                    xev.xclient.data.l[0] = windowOnTop ? 1 : 0; // _NET_WM_STATE_ADD = 1, _REMOVE = 0
                    xev.xclient.data.l[1] = wm_above;
                    xev.xclient.data.l[2] = 0;
                    xev.xclient.data.l[3] = 0;
                    xev.xclient.data.l[4] = 0;

                    XSendEvent(display, DefaultRootWindow(display), False,
                            SubstructureRedirectMask | SubstructureNotifyMask, &xev);
                    XFlush(display);
                    XCloseDisplay(display);
                }
#endif
                lastWindowOnTop = windowOnTop;
        }

        // Dragging the window for windows.
#ifdef _WIN32
        Vector2 mousePos = GetMousePosition();
        Vector2 windowPos = GetWindowPosition();
        Vector2 mouseScreenPos = {windowPos.x + mousePos.x, windowPos.y + mousePos.y};

        if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON) && !decorated_window)
        {
            isDragging = true;
            // Store the offset from window position to mouse in screen coordinates
            dragOffset.x = mouseScreenPos.x - windowPos.x;
            dragOffset.y = mouseScreenPos.y - windowPos.y;
        }

        if (IsMouseButtonReleased(MOUSE_RIGHT_BUTTON) && !decorated_window) {
            isDragging = false;
        }

        if (isDragging) {
            // Set window position based on screen mouse position minus offset
            SetWindowPosition(
                (int)(mouseScreenPos.x - dragOffset.x),
                (int)(mouseScreenPos.y - dragOffset.y)
            );
        }
#endif
        BeginDrawing();
        ClearBackground(DARKGRAY);

        // Begin ImGui frame
        rlImGuiBegin();

        //Updates the imgui window.
        UpdateUI();

        // End ImGui frame
        rlImGuiEnd();
        EndDrawing();
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // no 100% cpu usage
    }

    // Cleanup
    SettingsHandler::SaveSettings();
    input.cleanup();
    rlImGuiShutdown();
    UnloadAllTextures();
    CloseWindow();
    return 0;
}
