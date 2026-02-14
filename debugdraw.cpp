#include "debugdraw.h"

// --------------------
// Helpers
// --------------------
static inline sf::Vector2f toSf(const SFMLDebugDrawContext* ctx, b2Vec2 v)
{
    return { v.x * ctx->pixelsPerMeter, v.y * ctx->pixelsPerMeter };
}

static inline sf::Color toColor(b2HexColor c, uint8_t alpha = 255)
{
    // b2HexColor is 0xRRGGBB in Box2D 3.x
    uint32_t x = static_cast<uint32_t>(c);
    sf::Color out;
    out.r = static_cast<uint8_t>((x >> 16) & 0xFF);
    out.g = static_cast<uint8_t>((x >> 8) & 0xFF);
    out.b = static_cast<uint8_t>((x >> 0) & 0xFF);
    out.a = alpha;
    return out;
}

static inline void drawLine(sf::RenderTarget& target, sf::Vector2f a, sf::Vector2f b, sf::Color c)
{
    sf::Vertex line[2] = { sf::Vertex{a, c}, sf::Vertex{b, c} };
    target.draw(line, 2, sf::PrimitiveType::Lines);
}

static inline void drawPolyline(sf::RenderTarget& target,
    const sf::Vector2f* pts, std::size_t n,
    sf::Color c, bool closed)
{
    if (n < 2) return;
    sf::VertexArray va(sf::PrimitiveType::LineStrip, closed ? n + 1 : n);
    for (std::size_t i = 0; i < n; ++i) va[i] = sf::Vertex{ pts[i], c };
    if (closed) va[n] = sf::Vertex{ pts[0], c };
    target.draw(va);
}

static inline void drawConvexFill(sf::RenderTarget& target,
    const sf::Vector2f* pts, std::size_t n,
    sf::Color fill)
{
    if (n < 3) return;
    sf::VertexArray va(sf::PrimitiveType::TriangleFan, n);
    for (std::size_t i = 0; i < n; ++i) va[i] = sf::Vertex{ pts[i], fill };
    target.draw(va);
}

static inline void drawCircleOutline(sf::RenderTarget& target,
    sf::Vector2f center, float radiusPx,
    sf::Color c)
{
    constexpr int SEG = 32;
    sf::Vector2f pts[SEG];
    for (int i = 0; i < SEG; ++i) {
        float a = (static_cast<float>(i) / SEG) * 2.f * 3.1415926535f;
        pts[i] = center + sf::Vector2f(std::cos(a) * radiusPx, std::sin(a) * radiusPx);
    }
    drawPolyline(target, pts, SEG, c, true);
}

static inline void drawCircleFill(sf::RenderTarget& target,
    sf::Vector2f center, float radiusPx,
    sf::Color fill)
{
    constexpr int SEG = 32;
    sf::VertexArray va(sf::PrimitiveType::TriangleFan, SEG + 2);
    va[0] = sf::Vertex{ center, fill };
    for (int i = 0; i <= SEG; ++i) {
        float a = (static_cast<float>(i) / SEG) * 2.f * 3.1415926535f;
        sf::Vector2f p = center + sf::Vector2f(std::cos(a) * radiusPx, std::sin(a) * radiusPx);
        va[i + 1] = sf::Vertex{ p, fill };
    }
    target.draw(va);
}

// Rotate a vector by b2Rot (Box2D 3.x typically stores cos/sin as q.c / q.s).
static inline b2Vec2 rotate(b2Rot q, b2Vec2 v)
{
    return b2Vec2{ q.c * v.x - q.s * v.y, q.s * v.x + q.c * v.y };
}

// --------------------
// All b2DebugDraw callbacks
// --------------------
static void SFML_DrawPolygon(const b2Vec2* vertices, int vertexCount, b2HexColor color, void* context)
{
    auto* ctx = static_cast<SFMLDebugDrawContext*>(context);
    if (!ctx || !ctx->target || !vertices || vertexCount < 2) return;

    std::vector<sf::Vector2f> pts(static_cast<std::size_t>(vertexCount));
    for (int i = 0; i < vertexCount; ++i) pts[i] = toSf(ctx, vertices[i]);

    drawPolyline(*ctx->target, pts.data(), pts.size(), toColor(color, 255), true);
}

static void SFML_DrawSolidPolygon(b2Transform transform,
    const b2Vec2* vertices, int vertexCount,
    float radius, b2HexColor color, void* context)
{
    (void)radius; // rounded corners; ignored in this simple renderer
    auto* ctx = static_cast<SFMLDebugDrawContext*>(context);
    if (!ctx || !ctx->target || !vertices || vertexCount < 3) return;

    std::vector<sf::Vector2f> pts(static_cast<std::size_t>(vertexCount));
    for (int i = 0; i < vertexCount; ++i) {
        b2Vec2 v = vertices[i];
        b2Vec2 w = transform.p + rotate(transform.q, v);
        pts[i] = toSf(ctx, w);
    }

    sf::Color fill = toColor(color, 60);
    sf::Color line = toColor(color, 255);

    drawConvexFill(*ctx->target, pts.data(), pts.size(), fill);
    drawPolyline(*ctx->target, pts.data(), pts.size(), line, true);
}

static void SFML_DrawCircle(b2Vec2 center, float radius, b2HexColor color, void* context)
{
    auto* ctx = static_cast<SFMLDebugDrawContext*>(context);
    if (!ctx || !ctx->target) return;
    drawCircleOutline(*ctx->target, toSf(ctx, center), radius * ctx->pixelsPerMeter, toColor(color, 255));
}

static void SFML_DrawSolidCircle(b2Transform transform, float radius, b2HexColor color, void* context)
{
    auto* ctx = static_cast<SFMLDebugDrawContext*>(context);
    if (!ctx || !ctx->target) return;

    sf::Vector2f center = toSf(ctx, transform.p);
    float rpx = radius * ctx->pixelsPerMeter;

    sf::Color fill = toColor(color, 60);
    sf::Color line = toColor(color, 255);

    drawCircleFill(*ctx->target, center, rpx, fill);
    drawCircleOutline(*ctx->target, center, rpx, line);

    // Draw axis line to show rotation (x-axis of transform)
    b2Vec2 axis = b2Vec2{ transform.q.c, transform.q.s };
    drawLine(*ctx->target, center, center + sf::Vector2f(axis.x, axis.y) * rpx, line);
}

static void SFML_DrawSolidCapsule(b2Vec2 p1, b2Vec2 p2, float radius, b2HexColor color, void* context)
{
    auto* ctx = static_cast<SFMLDebugDrawContext*>(context);
    if (!ctx || !ctx->target) return;

    sf::Vector2f a = toSf(ctx, p1);
    sf::Vector2f b = toSf(ctx, p2);
    float rpx = radius * ctx->pixelsPerMeter;

    sf::Color fill = toColor(color, 60);
    sf::Color line = toColor(color, 255);

    sf::Vector2f d = b - a;
    float len = std::sqrt(d.x * d.x + d.y * d.y);
    if (len < 0.0001f) {
        drawCircleFill(*ctx->target, a, rpx, fill);
        drawCircleOutline(*ctx->target, a, rpx, line);
        return;
    }

    sf::Vector2f u = d / len;
    sf::Vector2f n{ -u.y, u.x };

    // rectangle body (approx)
    sf::Vector2f pA = a + n * rpx;
    sf::Vector2f pB = b + n * rpx;
    sf::Vector2f pC = b - n * rpx;
    sf::Vector2f pD = a - n * rpx;

    // fill rectangle
    {
        sf::VertexArray va(sf::PrimitiveType::Triangles, 6);
        va[0] = sf::Vertex{ pA, fill }; va[1] = sf::Vertex{ pB, fill }; va[2] = sf::Vertex{ pC, fill };
        va[3] = sf::Vertex{ pA, fill }; va[4] = sf::Vertex{ pC, fill }; va[5] = sf::Vertex{ pD, fill };
        ctx->target->draw(va);
    }

    // fill ends
    drawCircleFill(*ctx->target, a, rpx, fill);
    drawCircleFill(*ctx->target, b, rpx, fill);

    // outline
    {
        sf::Vector2f pts[4] = { pA, pB, pC, pD };
        drawPolyline(*ctx->target, pts, 4, line, true);
    }
    drawCircleOutline(*ctx->target, a, rpx, line);
    drawCircleOutline(*ctx->target, b, rpx, line);
    drawLine(*ctx->target, a, b, line);
}

static void SFML_DrawSegment(b2Vec2 p1, b2Vec2 p2, b2HexColor color, void* context)
{
    auto* ctx = static_cast<SFMLDebugDrawContext*>(context);
    if (!ctx || !ctx->target) return;
    drawLine(*ctx->target, toSf(ctx, p1), toSf(ctx, p2), toColor(color, 255));
}

static void SFML_DrawTransform(b2Transform transform, void* context)
{
    auto* ctx = static_cast<SFMLDebugDrawContext*>(context);
    if (!ctx || !ctx->target) return;

    sf::Vector2f p = toSf(ctx, transform.p);

    // axis lines in pixels
    const float axisLenPx = 20.f;

    // x-axis = (c, s); y-axis = (-s, c)
    sf::Vector2f xAxis(transform.q.c, transform.q.s);
    sf::Vector2f yAxis(-transform.q.s, transform.q.c);

    drawLine(*ctx->target, p, p + xAxis * axisLenPx, sf::Color(255, 80, 80));
    drawLine(*ctx->target, p, p + yAxis * axisLenPx, sf::Color(80, 255, 80));
}

static void SFML_DrawPoint(b2Vec2 p, float size, b2HexColor color, void* context)
{
    auto* ctx = static_cast<SFMLDebugDrawContext*>(context);
    if (!ctx || !ctx->target) return;

    sf::Vector2f pos = toSf(ctx, p);
    float r = std::max(1.f, size * 0.5f); // interpret as pixels

    drawCircleFill(*ctx->target, pos, r, toColor(color, 255));
}

// --------------------
// Setup function
// --------------------
void SetupB2DebugDraw_SFML(b2DebugDraw& outDraw, SFMLDebugDrawContext& ctx)
{
    outDraw = b2DefaultDebugDraw(); // start with defaults
    outDraw.context = &ctx;

    outDraw.DrawPolygonFcn = &SFML_DrawPolygon;
    outDraw.DrawSolidPolygonFcn = &SFML_DrawSolidPolygon;
    outDraw.DrawCircleFcn = &SFML_DrawCircle;
    outDraw.DrawSolidCircleFcn = &SFML_DrawSolidCircle;
    outDraw.DrawSolidCapsuleFcn = &SFML_DrawSolidCapsule;
    outDraw.DrawSegmentFcn = &SFML_DrawSegment;
    outDraw.DrawTransformFcn = &SFML_DrawTransform;
    outDraw.DrawPointFcn = &SFML_DrawPoint;

    // choose what to draw
    outDraw.drawShapes = true;
    outDraw.drawJoints = true;
    outDraw.drawBounds = false;
    outDraw.drawMass = false;
}
