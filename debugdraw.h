#pragma once
#include <box2d/box2d.h>
#include <SFML/Graphics.hpp>

#include <cmath>
#include <cstdint>
#include <limits>
#include <vector>

// --------------------
// Context you provide
// --------------------
struct SFMLDebugDrawContext
{
    sf::RenderTarget* target;
    float pixelsPerMeter;
};

void SetupB2DebugDraw_SFML(b2DebugDraw& outDraw, SFMLDebugDrawContext& ctx);