#include "Util.h"
#include <PCH.h>

#include "Input.h"
#include "Rendering/Renderer.h"

#include <SDL3/SDL.h>


vec3 Velox::toShaderCoords(vec3 vector, bool flipY = false)
{
    ivec2 windowSize = Velox::getWindowSize();

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

bool Velox::isOverlapping(const Velox::Rectangle& rectA, const Velox::Rectangle& rectB)
{
    float leftA   = rectA.x;
    float rightA  = rectA.x + rectA.w;
    float topA    = rectA.y;
    float bottomA = rectA.y + rectA.h;

    float leftB   = rectB.x;
    float rightB  = rectB.x + rectB.w;
    float topB    = rectB.y;
    float bottomB = rectB.y + rectB.h;

    bool overlapX = leftA < rightB  && rightA  > leftB;
    bool overlapY = topA  < bottomB && bottomA > topB;

    return overlapX && overlapY;
}

bool Velox::isMouseInArea(const Velox::Rectangle& rect)
{
    vec2 mousePosition = Velox::getMousePosition();
    mousePosition.y = Velox::getWindowSize().y - mousePosition.y;
    
    if (mousePosition.x < rect.x || mousePosition.x > rect.x + rect.w)
        return false;

    if (mousePosition.y < rect.y || mousePosition.y > rect.y + rect.h)
        return false;

    return true;
}

