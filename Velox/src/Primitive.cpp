#include "Primitive.h"

#include "Renderer.h"


void Velox::DrawRectangle(vec4 rectangle, vec4 color, int textureIndex)
{
    vec2 origin = vec2(rectangle.x, rectangle.y);
    vec2 size   = vec2(rectangle.z, rectangle.w);

    vec3 topLeft  = vec3(origin.x,          origin.y,          0.0);
    vec3 topRight = vec3(origin.x + size.x, origin.y,          0.0);
    vec3 botLeft  = vec3(origin.x,          origin.y + size.y, 0.0);
    vec3 botRight = vec3(origin.x + size.x, origin.y + size.y, 0.0);
    
    //                                       | vert | color | uv | tex_index |
    Uint32 topLeftIndex  = Velox::AddVertex({ topLeft,  color, vec2(0, 0), textureIndex });
    Uint32 topRightIndex = Velox::AddVertex({ topRight, color, vec2(1, 0), textureIndex });
    Uint32 botLeftIndex  = Velox::AddVertex({ botLeft,  color, vec2(0, 1), textureIndex });
    Uint32 botRightIndex = Velox::AddVertex({ botRight, color, vec2(1, 1), textureIndex });

    // Connect the dots.
    Velox::AddIndex(topLeftIndex);
    Velox::AddIndex(topRightIndex);
    Velox::AddIndex(botLeftIndex);
    Velox::AddIndex(botLeftIndex);
    Velox::AddIndex(topRightIndex);
    Velox::AddIndex(botRightIndex);
}
