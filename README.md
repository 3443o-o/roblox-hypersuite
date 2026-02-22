<img width="64" height="64" alt="logo" src="https://github.com/user-attachments/assets/ae94bb93-313c-401a-981d-1b73a47b365b" /> 

# Roblox Hypersuite

> [!WARNING]
> This repo is EXTREMELY OUTDATED, new repo at: https://gitlab.com/3443e/roblox-hypersuite

A cross-platform multi-purpose Roblox utility with advanced scripting capabilities.

## Features

### Built-in Macros
A comprehensive collection of useful macros with detailed tutorials for glitching in R6 and R15 games:

- **Freeze** - Temporarily freeze the Roblox process
- **Extended laugh clip** - Extended character animation clipping
- **Extended dance clip** - Dance animation exploitation
- **Buckey clip** - Advanced clipping technique
- **Speed glitch** - Movement speed manipulation
- **Spam key** - Rapid key input automation
- **Disable head collision** - Remove head hitbox collision
- **No head collision roof clip** - Clip through ceilings using head collision removal
- **Helicopter high jump** - Enhanced jumping technique
- **Full gear desync (dropping)** - Gear desynchronization exploit

All macros include comprehensive in-app explanations and tutorials.

### Network Control
- **Customizable lag switch** - Simulate network lag with configurable packet loss and ping
- **Ping increasor** - Add artificial latency to your connection

### Utility Tools
- **XML Editor** - Edit `GlobalBasicSettings` and other Roblox configuration files
- **Rejoin System** - Change FPS settings and rejoin the same server
- **Settings Persistence** - Save and restore your configurations
- **Themable UI** - Customize the interface appearance

### Custom Scripting System
Create and import your own macros using Lua scripts (`.hss` files):

- **Lua-based scripting** - Write custom automation scripts
- **Keyboard & Mouse Control** - Simulate inputs programmatically
- **Process Control** - Freeze/unfreeze Roblox process
- **Network Manipulation** - Control lag and packet loss via scripts
- **Persistent Scripts** - Scripts are saved and loaded automatically
- **Keybind Support** - Assign custom keybinds to your scripts
- **Hot Reloading** - Reload scripts without restarting the application

#### Example Script
```lua
-- @name: Jump Spam
-- @desc: Rapidly presses space bar 10 times
-- @author: Example
-- @version: 1.0
-- @keybind: F6

function onExecute()
    for i = 1, 10 do
        pressKey("Space", 50)
        sleep(50)
    end
    log("Jump spam complete!")
end
```

See the [Scripting Documentation](https://3443o-o.github.io/hypersuite/Docs) for complete API reference and examples.

## Installation

1. Download the latest release from the [Releases](https://github.com/3443o-o/roblox-hypersuite/releases) page
2. Extract the archive
3. Run the executable
4. (Optional) Place custom `.hss` scripts in the `scripts/` folder for automatic loading

## Platform Support

- Linux (primary)
- Windows (via MinGW cross-compilation)

## Building from Source

### Prerequisites

**For Linux builds:**
- `g++` with C++17 support
- `make`
- `raylib` development libraries
- Lua 5.4 (included in `include/lua-5.4.2_linux/`)
- OpenGL development libraries
- X11 development libraries
- Optional: `ccache` for faster recompilation

Install dependencies on Ubuntu/Debian:
```bash
sudo apt-get install build-essential libraylib-dev libgl1-mesa-dev libx11-dev ccache
```

**For Windows cross-compilation (from Linux):**
- `x86_64-w64-mingw32-g++` (MinGW-w64 cross-compiler)
- Windows raylib libraries (included in `include/raylibWin64/`)
- Windows Lua 5.4 libraries (included in `include/lua-5.4.2_win64/`)

Install MinGW on Ubuntu/Debian:
```bash
sudo apt-get install mingw-w64
```

### Build Instructions

**Build for Linux:**
```bash
git clone https://github.com/yourusername/roblox-hypersuite.git
cd roblox-hypersuite
make linux
# Output: build/linux/utility
```

**Build for Windows (cross-compile from Linux):**
```bash
make windows
# Output: build/win64/utility.exe
```

**Build both platforms:**
```bash
make all linux windows
```

**Additional build options:**
```bash
make debug          # Debug build with symbols and no optimization
make lto            # Link-time optimization for better performance
make clean          # Clean all builds
make clean-linux    # Clean Linux build only
make clean-windows  # Clean Windows build only
make info           # Show source and object files
```

### Build System Features

- Parallel compilation (uses all CPU cores)
- Automatic dependency tracking
- Incremental builds (only recompiles changed files)
- ccache support for faster recompilation
- Static linking for standalone executables

## Usage

1. Launch Roblox Hypersuite
2. Select desired macros from the list
3. Configure keybinds by right-clicking on macros
4. Enable/disable macros with right-click
5. Import custom scripts using the "+ Import script" button

## Scripting API

### Core Functions
- `pressKey(key, delay)` - Press and release a key
- `holdKey(key)` - Hold a key down
- `releaseKey(key)` - Release a held key
- `typeText(text, delay)` - Type a string
- `moveMouse(dx, dy)` - Move mouse cursor
- `sleep(ms)` - Pause execution
- `log(message)` - Print to console

### Advanced Functions
- `robloxFreeze(enable)` - Freeze/unfreeze Roblox process
- `lagSwitchMan(enable, packet_loss, ping_lag)` - Control network simulation
- `isKeyPressed(key)` - Check key state
- `waitForKey(timeout)` - Wait for key input

### Global Variables
- `HSPlayerName` - Current Roblox username
- `HSPlayerDisplayName` - Current Roblox display name

For complete documentation, visit the [scripting guide](https://3443o-o.github.io/hypersuite/Docs).

## Security Warning

**Only import scripts from trusted sources.** Custom scripts have access to:
- Keyboard and mouse simulation
- Process control (suspend/resume)
- Network manipulation

Always review script code before importing.

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for bugs and feature requests.

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## License

GNU General Public License v3.0 - See [LICENSE](LICENSE) file for details.

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

## Credits

This project uses the following open-source libraries:

- [Spencer Macro Utilities](https://github.com/Spencer0187/Spencer-Macro-Utilities) - Original macro inspiration
- [Fumble](https://github.com/bornacvitanic/fumble) - Network control implementation
- [raylib](https://github.com/raysan5/raylib) - Graphics and windowing
- [rlImGui](https://github.com/raylib-extras/rlImGui) - ImGui integration for raylib
- [Dear ImGui](https://github.com/ocornut/imgui) - Immediate mode GUI library
- [pugixml](https://github.com/zeux/pugixml) - XML parsing
- [JSON for Modern C++](https://github.com/nlohmann/json) - JSON parsing
- [Lua](https://www.lua.org/) - Scripting language

## Disclaimer

This tool is for educational purposes only. Use at your own risk. The developers are not responsible for any consequences of using this software, including but not limited to account bans or other penalties imposed by Roblox.

## Support

For support, documentation, and updates:
- Visit our [website](https://3443o-o.github.io/hypersuite/)
- Join our community [Discord/Forum]
- Check the [documentation](https://3443o-o.github.io/hypersuite/Docs)

---

Made by the Roblox Hypersuite team | Licensed under GPL-3.0
