#include "LoadTextures.hpp"
#include "raylib.h"
#include "Globals.hpp"
#include <vector>

std::vector<Texture2D> LoadedTextures;

Texture2D LoadTextureFromFile(const char* filename) {
    Texture2D texture = LoadTexture(filename);
    SetTextureFilter(texture, TEXTURE_FILTER_POINT);
    LoadedTextures.push_back(texture);
    return texture;
}

void UnloadAllTextures() {
    for (auto& texture : LoadedTextures) {
        UnloadTexture(texture);
    }
    LoadedTextures.clear();
}

void registerTexture(Texture2D texture) {
    LoadedTextures.push_back(texture);
}

void LoadAllSprites() {
    LoadTextureFromFile("resources/e-dance-clip.jpg"); // 0
    LoadTextureFromFile("resources/laugh.jpg"); // 1
    LoadTextureFromFile("resources/buckey.jpg"); // 2
    LoadTextureFromFile("resources/Gear-clip.jpg"); // 3
    LoadTextureFromFile("resources/no-head.jpg"); // 4
    LoadTextureFromFile("resources/nhc-roof.jpg"); // 5
    LoadTextureFromFile("resources/fullgeardesync.png"); // 6
}
