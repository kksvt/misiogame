#include "TextureManager.h"
#include <iostream>

bool TextureManager::load_texture(const std::string& path)
{
    const auto &it = texture_map.find(path);
    if (it != texture_map.end()) {
        std::cerr << "Texture [" << path << "] is already loaded\n";
        return true;
    }

    auto& tex = texture_map.try_emplace(path).first->second;
    if (!tex.loadFromFile(path)) {
        std::cerr << "Texture [" << path << "] not found\n";
        texture_map.erase(path);
        return false;
    }
    return true;
}

const sf::Texture* TextureManager::get_texture(const std::string& name)
{
    const auto &it = texture_map.find(name);
    if (it == texture_map.end()) {
        return nullptr;
    }
    return &it->second;
}

std::unordered_map<std::string, sf::Texture> TextureManager::texture_map;