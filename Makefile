# Parallel builds
MAKEFLAGS += -j$(shell nproc)

# Compiler optimization flags
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra
# Use ccache if available for faster recompilation
CXX := $(shell command -v ccache >/dev/null 2>&1 && echo "ccache g++" || echo "g++")

# Precompiled header support
PCH_SRC = src/headers/pch.hpp
PCH_OUT = $(PCH_SRC).gch

# Source files
SRCS = $(shell find src -name "*.cpp") \
       include/imgui/imgui.cpp \
       include/imgui/imgui_draw.cpp \
       include/imgui/imgui_tables.cpp \
       include/imgui/imgui_widgets.cpp \
       include/rlImGui/rlImGui.cpp \
       include/pugixml/pugixml.cpp

# Include directories
INCLUDES = -I./include \
           -I./src/headers/obstructive \
           -I./src/headers \
           -I./include/imgui \
           -I./include/rlImGui \
           -I./include/pugixml

# -------------------------------------------------------------------
# Linux build
# -------------------------------------------------------------------
LINUX_OBJ_DIR = out/linux
LINUX_TARGET = build/linux/utility
LINUX_LDFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
LINUX_OBJS = $(patsubst %.cpp,$(LINUX_OBJ_DIR)/%.o,$(SRCS))
LINUX_DEPS = $(LINUX_OBJS:.o=.d)

.PHONY: all linux clean clean-linux clean-windows windows

all: linux

linux: $(LINUX_TARGET)

$(LINUX_TARGET): $(LINUX_OBJS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LINUX_LDFLAGS) -static-libgcc -static-libstdc++

$(LINUX_OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -MMD -MP -c $< -o $@

# Include dependency files
-include $(LINUX_DEPS)

# -------------------------------------------------------------------
# Windows build
# -------------------------------------------------------------------
WIN_OBJ_DIR = out/win64
WIN_TARGET = build/win64/utility.exe
WIN_CXX_BASE = x86_64-w64-mingw32-g++
WIN_CXX := $(shell command -v ccache >/dev/null 2>&1 && echo "ccache $(WIN_CXX_BASE)" || echo "$(WIN_CXX_BASE)")
WIN_CXXFLAGS = -std=c++17 -O2 -Wall -Wextra $(INCLUDES) -I./include/raylibWin64/include
WIN_LDFLAGS = -L./include/raylibWin64/lib -static -lraylib -lopengl32 -lgdi32 -lwinmm \
              -static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -mwindows -lshell32 -lpthread -Wl,-Bdynamic
WIN_ICON = resources/icon.o
WIN_MANIFEST = resources/app.res
WIN_OBJS = $(patsubst %.cpp,$(WIN_OBJ_DIR)/%.o,$(SRCS))
WIN_DEPS = $(WIN_OBJS:.o=.d)

windows: $(WIN_TARGET)

$(WIN_OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(WIN_CXX) $(WIN_CXXFLAGS) -MMD -MP -c $< -o $@

$(WIN_TARGET): $(WIN_OBJS) $(WIN_ICON) $(WIN_MANIFEST)
	@mkdir -p $(dir $@)
	$(WIN_CXX) -o $@ $(WIN_OBJS) $(WIN_ICON) $(WIN_MANIFEST) $(WIN_LDFLAGS)

# Include dependency files
-include $(WIN_DEPS)

# -------------------------------------------------------------------
# Clean targets
# -------------------------------------------------------------------
clean: clean-linux clean-windows

clean-linux:
	rm -rf build/linux out/linux

clean-windows:
	rm -rf build/win64 out/win64

# -------------------------------------------------------------------
# Additional optimization targets
# -------------------------------------------------------------------
# Link-time optimization build (slower compile, faster runtime)
lto: CXXFLAGS += -flto
lto: LINUX_LDFLAGS += -flto
lto: linux

# Debug build without optimizations
debug: CXXFLAGS = -std=c++17 -g -O0 -Wall -Wextra
debug: linux
