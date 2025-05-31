#include "Primitive.h"

#include "Renderer.h"
#include "Util.h"


void Velox::DrawRectangle(vec4 rectangle, vec4 color)
{
    vec2 origin = vec2(rectangle.x, rectangle.y);
    vec2 size   = vec2(rectangle.z, rectangle.w);

    vec3 topLeft  = Velox::ToShaderCoords(vec3(origin.x,          origin.y,          0.0), true);
    vec3 topRight = Velox::ToShaderCoords(vec3(origin.x + size.x, origin.y,          0.0), true);
    vec3 botLeft  = Velox::ToShaderCoords(vec3(origin.x,          origin.y + size.y, 0.0), true);
    vec3 botRight = Velox::ToShaderCoords(vec3(origin.x + size.x, origin.y + size.y, 0.0), true);
    
    //                                                     | vert | color | uv |
    Uint32 topLeftIndex  = Velox::AddVertex(Velox::Vertex { topLeft,  color, { 0, 0 } });
    Uint32 topRightIndex = Velox::AddVertex(Velox::Vertex { topRight, color, { 1, 0 } });
    Uint32 botLeftIndex  = Velox::AddVertex(Velox::Vertex { botLeft,  color, { 0, 1 } });
    Uint32 botRightIndex = Velox::AddVertex(Velox::Vertex { botRight, color, { 1, 1 } });

    // Connect the dots.
    Velox::AddIndex(topLeftIndex);
    Velox::AddIndex(topRightIndex);
    Velox::AddIndex(botLeftIndex);
    Velox::AddIndex(botLeftIndex);
    Velox::AddIndex(topRightIndex);
    Velox::AddIndex(botRightIndex);
}
