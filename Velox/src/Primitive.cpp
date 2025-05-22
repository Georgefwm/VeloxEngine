#include "Primitive.h"

#include "Renderer.h"
#include "Util.h"


void Velox::DrawRectangle(vec4 rectangle, vec4 color)
{
    vec2 origin = vec2(rectangle.x, rectangle.y);
    vec2 size   = vec2(rectangle.z, rectangle.w);

    vec3 topLeft  = Velox::toShaderCoords(vec3(origin.x,          origin.y, 0.0), true);
    vec3 topRight = Velox::toShaderCoords(vec3(origin.x + size.x, origin.y, 0.0), true);

    vec3 botLeft  = Velox::toShaderCoords(vec3(origin.x,          origin.y + size.y, 0.0), true);
    vec3 botRight = Velox::toShaderCoords(vec3(origin.x + size.x, origin.y + size.y, 0.0), true);
    
    // Vertices must be added in clockwise order!
    // First triangle.
    AddVertex(Velox::Vertex { botLeft,  color });
    AddVertex(Velox::Vertex { topLeft,  color });
    AddVertex(Velox::Vertex { topRight, color });

    // Second.
    AddVertex(Velox::Vertex { topRight, color });
    AddVertex(Velox::Vertex { botRight, color });
    AddVertex(Velox::Vertex { botLeft,  color });
}
