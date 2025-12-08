#ifndef LOADTEXTURES_HPP
#define LOADTEXTURES_HPP
#include "raylib.h"
#include <vector>

extern std::vector<Texture2D> LoadedTextures;
Texture2D LoadTextureFromFile(const char* filename);
void UnloadAllTextures();
void registerTexture(Texture2D texture);
void LoadAllSprites();

#endif