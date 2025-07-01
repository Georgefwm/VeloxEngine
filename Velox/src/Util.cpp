#include "Util.h"
#include <PCH.h>

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


