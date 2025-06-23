#include "Primitive.h"
#include <PCH.h>

#include "Renderer.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

vec3 Rotate(mat4 transform, vec3 position)
{
    glm::vec4 pos = glm::vec4(position, 1.0f);
    pos = transform * pos;
    return glm::vec3(pos);
}

void Velox::DrawRectangle(vec4 rectangle, vec4 color, f32 rotation, unsigned int textureId, unsigned int shaderId)
{
    vec2 origin = vec2(rectangle.x, rectangle.y);
    vec2 size   = vec2(rectangle.z, rectangle.w);

    vec3 positions[4];
    positions[0] = vec3(origin.x,          origin.y,          0.0);
    positions[1] = vec3(origin.x + size.x, origin.y,          0.0);
    positions[2] = vec3(origin.x,          origin.y + size.y, 0.0);
    positions[3] = vec3(origin.x + size.x, origin.y + size.y, 0.0);

    if (rotation != 0)
    {
        glm::vec3 center(0.0f);
        for (int i = 0; i < 4; i++)
            center += positions[i];

        center /= static_cast<float>(4);

        glm::mat4 transform(1.0f);
        transform = glm::translate(transform, center);                                  // Move to center
        transform = glm::rotate(transform, glm::radians(rotation), glm::vec3(0, 0, 1)); // Rotate around Z
        transform = glm::translate(transform, -center);                                 // Move back

        for (int i = 0; i < 4; i++)
            positions[i] = glm::vec3(transform * glm::vec4(positions[i], 1.0f));
    }
    
    std::vector<Velox::Vertex> vertices(4);
    vertices[0] = { positions[0], color, vec2(0, 0) };
    vertices[1] = { positions[1], color, vec2(1, 0) };
    vertices[2] = { positions[2], color, vec2(0, 1) };
    vertices[3] = { positions[3], color, vec2(1, 1) };

    std::vector<unsigned int> indices = { 0, 1, 2, 2, 1, 3 };

    Velox::Draw(vertices, indices, textureId, shaderId);
}




