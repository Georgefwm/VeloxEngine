#include "Util.h"

#include "Renderer.h"

#include <SDL3/SDL.h>

static uint64_t g_lastTime = 0;
static double g_deltaTime  = 0;  // Stored as seconds.

vec3 Velox::ToShaderCoords(vec3 vector, bool flipY = false)
{
    ivec2 windowSize = Velox::GetWindowSize();

    // Convert from pixel space to [0, 1]
    vec2 normalisedXY = {
        vector.x / windowSize.x,
        vector.y / windowSize.y
    };

    // Map to [-1, 1]
    normalisedXY = normalisedXY * 2.0f - 1.0f;

    // Flip Y-axis if needed (OpenGL-style NDC)
    if (flipY) 
        normalisedXY.y *= -1.0f;

    return glm::vec3(normalisedXY, vector.z);
}

float Velox::DeltaTime()
{
    return (float)g_deltaTime;
}

double Velox::DeltaTimePrecise()
{
    return g_deltaTime;
}

float Velox::DeltaTimeMS()
{
    return (float)(g_deltaTime * SDL_MS_PER_SECOND);
}

// In ms.
float Velox::Time()
{
    return (float)g_lastTime;
}

float Velox::RefreshDeltaTime()
{
    uint64_t nowTime = SDL_GetTicks();

    g_deltaTime = (nowTime - g_lastTime) / (double)SDL_MS_PER_SECOND;

    g_lastTime = nowTime;
    
    return DeltaTime();
}

