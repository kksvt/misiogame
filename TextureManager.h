#pragma once

#include <SFML//Graphics.hpp>
#include <unordered_map>
#include <string>

struct TextureManager
{
public:
	static bool load_texture(const std::string& path);
	static const sf::Texture* get_texture(const std::string& name);
private:
	TextureManager();
	static std::unordered_map<std::string, sf::Texture> texture_map;


};
