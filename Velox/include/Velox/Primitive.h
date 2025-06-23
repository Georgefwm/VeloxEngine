#pragma once

namespace Velox {

// Texture index of -1 signifies "no texture, only use color".
void DrawRectangle(vec4 rectangle, vec4 color, f32 rotation = 0,
        unsigned int textureIndex = 0, unsigned int shaderId = 0);


}
